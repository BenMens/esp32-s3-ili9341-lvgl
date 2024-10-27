#include <string.h>
#include <sx127x.h>

#include "df-player.hpp"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "driver/uart.h"
#include "esp-display.hpp"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_log.h"
#include "gui.hpp"
#include "neopixel.hpp"

#define TAG "main"

#define NUM_LEDS 1

#define MP3_RX 15
#define MP3_TX 7

static Pixels *neoPixels = NULL;

#if CONFIG_PRJ_LORA_SPI1_HOST == 1
#define LORA_SPI SPI1_HOST
#elif CONFIG_PRJ_LORA_SPI2_HOST == 1
#define LORA_SPI SPI2_HOST
#elif CONFIG_PRJ_LORA_SPI3_HOST == 1
#define LORA_SPI SPI3_HOST
#endif

struct LoraMsg {
    int id;
    int counter;
};

static adc_oneshot_unit_handle_t oneshot_handle;

sx127x *sx127xDevice = NULL;
TaskHandle_t handle_interrupt;
int messages_sent = 0;
int supported_power_levels[] = {
    2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 20,
};
int supported_power_levels_count = sizeof(supported_power_levels) / sizeof(int);
int current_power_level = 0;

void IRAM_ATTR handle_interrupt_fromisr(void *arg)
{
    xTaskResumeFromISR(handle_interrupt);
}

void handle_interrupt_task(void *arg)
{
    while (1) {
        vTaskSuspend(NULL);
        sx127x_handle_interrupt((sx127x *)arg);
    }
}

void tx_callback(sx127x *device)
{
    if (messages_sent > 0) {
        ESP_LOGI(TAG, "transmitted");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    LoraMsg msg = {
        .id = 42,
        .counter = messages_sent,
    };

    ESP_ERROR_CHECK(sx127x_lora_tx_set_for_transmission((uint8_t *)&msg,
                                                        sizeof(msg), device));

    ESP_ERROR_CHECK(
        sx127x_set_opmod(SX127x_MODE_TX, SX127x_MODULATION_LORA, device));
    ESP_LOGI(TAG, "transmitting message %u", messages_sent);

    messages_sent++;
}

void cad_callback(sx127x *device, int cad_detected)
{
    if (cad_detected == 0) {
        ESP_LOGI(TAG, "cad not detected");
        ESP_ERROR_CHECK(
            sx127x_set_opmod(SX127x_MODE_CAD, SX127x_MODULATION_LORA, device));
        return;
    }
    // put into RX mode first to handle interrupt as soon as possible
    ESP_ERROR_CHECK(
        sx127x_set_opmod(SX127x_MODE_RX_CONT, SX127x_MODULATION_LORA, device));
    ESP_LOGI(TAG, "cad detected\n");
}

void setupLora()
{
    spi_device_interface_config_t dev_cfg;
    memset(&dev_cfg, 0, sizeof(dev_cfg));
    dev_cfg.command_bits = 0;
    dev_cfg.address_bits = 8;
    dev_cfg.dummy_bits = 0;
    dev_cfg.mode = 0;
    dev_cfg.clock_source = SPI_CLK_SRC_DEFAULT;
    dev_cfg.clock_speed_hz = 1000000;
    dev_cfg.spics_io_num = (gpio_num_t)(CONFIG_PRJ_LORA_PIN_SS);
    dev_cfg.queue_size = 16;

    spi_device_handle_t lora_spi_device;
    ESP_ERROR_CHECK(spi_bus_add_device(LORA_SPI, &dev_cfg, &lora_spi_device));

    ESP_ERROR_CHECK(sx127x_create(lora_spi_device, &sx127xDevice));

    // uint8_t registers[0x80];
    // sx127x_dump_registers(registers, sx127xDevice);

    // for (unsigned int i = 0; i < 0x80; i++) {
    //     ESP_LOGI(TAG, "%02x = %02x", i, registers[i]);
    // }

    ESP_ERROR_CHECK(sx127x_set_opmod(SX127x_MODE_SLEEP, SX127x_MODULATION_LORA,
                                     sx127xDevice));
    ESP_ERROR_CHECK(sx127x_set_frequency(865123456, sx127xDevice));

    ESP_ERROR_CHECK(sx127x_lora_reset_fifo(sx127xDevice));
    ESP_ERROR_CHECK(sx127x_set_opmod(SX127x_MODE_STANDBY,
                                     SX127x_MODULATION_LORA, sx127xDevice));
    ESP_ERROR_CHECK(sx127x_lora_set_bandwidth(SX127x_BW_125000, sx127xDevice));
    ESP_ERROR_CHECK(sx127x_lora_set_implicit_header(NULL, sx127xDevice));
    ESP_ERROR_CHECK(sx127x_lora_set_modem_config_2(SX127x_SF_9, sx127xDevice));
    ESP_ERROR_CHECK(sx127x_lora_set_syncword(18, sx127xDevice));
    ESP_ERROR_CHECK(sx127x_set_preamble_length(8, sx127xDevice));
    sx127x_tx_set_callback(tx_callback, sx127xDevice);

    BaseType_t task_code = xTaskCreatePinnedToCore(
        handle_interrupt_task, "handle interrupt", 8196, sx127xDevice, 2,
        &handle_interrupt, xPortGetCoreID());
    if (task_code != pdPASS) {
        ESP_LOGE(TAG, "can't create task %d", task_code);
        sx127x_destroy(sx127xDevice);
        return;
    }

    gpio_set_direction((gpio_num_t)CONFIG_PRJ_LORA_PIN_DIO0, GPIO_MODE_INPUT);
    gpio_pulldown_en((gpio_num_t)CONFIG_PRJ_LORA_PIN_DIO0);
    gpio_pullup_dis((gpio_num_t)CONFIG_PRJ_LORA_PIN_DIO0);
    gpio_set_intr_type((gpio_num_t)CONFIG_PRJ_LORA_PIN_DIO0, GPIO_INTR_POSEDGE);
    gpio_isr_handler_add((gpio_num_t)CONFIG_PRJ_LORA_PIN_DIO0,
                         handle_interrupt_fromisr, (void *)sx127xDevice);

    ESP_ERROR_CHECK(
        sx127x_tx_set_pa_config(SX127x_PA_PIN_BOOST, 13, sx127xDevice));
    sx127x_tx_header_t header = {.enable_crc = true,
                                 .coding_rate = SX127x_CR_4_5};
    ESP_ERROR_CHECK(sx127x_lora_tx_set_explicit_header(&header, sx127xDevice));

    tx_callback(sx127xDevice);
}

static int scanI2CBus()
{
    uint8_t address;
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");
    for (int i = 0; i < 128; i += 16) {
        printf("%02x: ", i);
        for (int j = 0; j < 16; j++) {
            fflush(stdout);
            address = i + j;

            i2c_cmd_handle_t cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, 0x01);
            i2c_master_stop(cmd);

            esp_err_t ret =
                i2c_master_cmd_begin(I2C_NUM_0, cmd, 50 / portTICK_PERIOD_MS);

            i2c_cmd_link_delete(cmd);

            if (ret == ESP_OK) {
                printf("%02x ", address);
            } else if (ret == ESP_ERR_TIMEOUT) {
                printf("UU ");
            } else {
                printf("-- ");
            }
        }
        printf("\r\n");
    }

    return 0;
}

void setupJoystick()
{
    adc_oneshot_unit_init_cfg_t init_config2 = {
        .unit_id = ADC_UNIT_2,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config2, &oneshot_handle));

    adc_oneshot_chan_cfg_t acd_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(oneshot_handle, ADC_CHANNEL_4, &acd_config));

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(oneshot_handle, ADC_CHANNEL_5, &acd_config));

    adc_cali_handle_t calibration_handle = NULL;
    esp_err_t ret = ESP_FAIL;

    ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_2,
        .chan = ADC_CHANNEL_4,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ret =
        adc_cali_create_scheme_curve_fitting(&cali_config, &calibration_handle);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");

    } else if (ret == ESP_ERR_NOT_SUPPORTED) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }
}

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_IRAM));

    neoPixels = new Pixels(GPIO_NUM_48, NUM_LEDS, Pixels::StripType::SK68XXMINI,
                           Pixels::ColorOrder::GRB, 2.8);

    neoPixels->setPixel(0, {
                               .red = 0x00,
                               .green = 0x00,
                               .blue = 0x08,
                               .white = 0x00,
                           });
    neoPixels->write();

    ESP_LOGI(TAG, "setup SPI2_HOST");
    spi_bus_config_t buscfg2 = {
        .mosi_io_num = (gpio_num_t)(CONFIG_PRJ_PIN_SPI2_MOSI),
        .miso_io_num = (gpio_num_t)(CONFIG_PRJ_PIN_SPI2_MISO),
        .sclk_io_num = (gpio_num_t)(CONFIG_PRJ_PIN_SPI2_SCK),
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .data4_io_num = 0,
        .data5_io_num = 0,
        .data6_io_num = 0,
        .data7_io_num = 0,
        .max_transfer_sz =
            24 * 240 * 2 + 8,  // TODO parameterize this based on LCD settings
        .flags = 0,
        .isr_cpu_id = (esp_intr_cpu_affinity_t)0,
        .intr_flags = 0,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg2, SPI_DMA_CH_AUTO));

    ESP_LOGI(TAG, "setup SPI3_HOST");
    spi_bus_config_t buscfg3 = {
        .mosi_io_num = (gpio_num_t)(CONFIG_PRJ_PIN_SPI3_MOSI),
        .miso_io_num = (gpio_num_t)(CONFIG_PRJ_PIN_SPI3_MISO),
        .sclk_io_num = (gpio_num_t)(CONFIG_PRJ_PIN_SPI3_SCK),
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .data4_io_num = 0,
        .data5_io_num = 0,
        .data6_io_num = 0,
        .data7_io_num = 0,
        .max_transfer_sz = 200000,
        .flags = 0,
        .isr_cpu_id = (esp_intr_cpu_affinity_t)0,
        .intr_flags = 0,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &buscfg3, SPI_DMA_CH_AUTO));

    setupLora();

    ESP_LOGI(TAG, "Initialize I2C bus");
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = (gpio_num_t)(CONFIG_PRJ_PIN_SDA),
        .scl_io_num = (gpio_num_t)(CONFIG_PRJ_PIN_SCL),
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master =
            {
                .clk_speed = CONFIG_PRJ_I2C_CLOCK_SPEED,
            },
        .clk_flags = 0,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));

    scanI2CBus();

    guiInit();

    // uart_config_t uart_config = {
    //     .baud_rate = 9600,
    //     .data_bits = UART_DATA_8_BITS,
    //     .parity = UART_PARITY_DISABLE,
    //     .stop_bits = UART_STOP_BITS_1,
    //     .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    //     .rx_flow_ctrl_thresh = 0,
    //     .source_clk = UART_SCLK_DEFAULT,
    //     .flags =
    //         {
    //             .backup_before_sleep = 0,
    //         },
    // };

    // Configure UART parameters
    // const int uart_buffer_size = (200);
    // QueueHandle_t uart_queue;
    // Install UART driver using an event queue here
    // ESP_ERROR_CHECK(
    //     uart_driver_install(UART_NUM_2, uart_buffer_size, 0, 0, NULL, 0));

    // ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));

    // ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, MP3_TX, MP3_RX, UART_PIN_NO_CHANGE,
    //                              UART_PIN_NO_CHANGE));

    // setupJoystick();

    // static int adc_raw[2];
    // static int voltage[2];

    // DFPlayerMini_Fast player;

    // player.begin(UART_NUM_2, false, 1000);

    // player.volume(10);

    // while (true) {
    //     player.playFromMP3Folder(15);
    //     player.query(0x42, 0, 0);

    //     // ESP_ERROR_CHECK(
    //     //     adc_oneshot_read(oneshot_handle, ADC_CHANNEL_4, &adc_raw[0]));
    //     // ESP_ERROR_CHECK(
    //     //     adc_oneshot_read(oneshot_handle, ADC_CHANNEL_5, &adc_raw[1]));
    //     // ESP_LOGI(TAG, "x=%d y=%d", adc_raw[0], adc_raw[1]);
    //     // if (calibration_handle) {
    //     //     // ESP_ERROR_CHECK(adc_cali_raw_to_voltage(calibration_handle,
    //     //     //                                         adc_raw[0],
    //     //     //                                         &voltage[0]));
    //     //     // ESP_ERROR_CHECK(adc_cali_raw_to_voltage(calibration_handle,
    //     //     //                                         adc_raw[1],
    //     //     //                                         &voltage[1]));
    //     //     ESP_LOGI(TAG, "x=%d mV y=%d mV", voltage[0], voltage[1]);
    //     // }

    //     vTaskDelay(10000 / portTICK_PERIOD_MS);
    // }

    vTaskDelay(2000 / portTICK_PERIOD_MS);
}
