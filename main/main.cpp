#include <stdio.h>
#include <string.h>
#include <sx127x.h>

#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_log.h"
#include "neopixel.hpp"
#include "esp-display.hpp"

#define TAG "main"

#define NUM_LEDS 1

static Pixels *neoPixels = NULL;

#define EXAMPLE_LCD_PIXEL_CLOCK_HZ (400 * 1000)

#define PIN_NUM_SDA (gpio_num_t)(8)
#define PIN_NUM_SCL (gpio_num_t)(9)

#define PIN_SPI3_MISO (gpio_num_t)(2)
#define PIN_SPI3_MOSI (gpio_num_t)(1)
#define PIN_SPI3_SCK (gpio_num_t)(21)

#define LORA_SPI SPI3_HOST
#define LORA_SS (gpio_num_t)(4)
#define LORA_RST (gpio_num_t)(5)

static sx127x *sx127xDevice = NULL;

#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define WRITE_BIT I2C_MASTER_WRITE  /*!< I2C master write */
#define ACK_CHECK_EN 0x1            /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0 /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0       /*!< I2C ack value */
#define NACK_VAL 0x1      /*!< I2C nack value */

static adc_oneshot_unit_handle_t oneshot_handle;

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
    dev_cfg.spics_io_num = LORA_SS;
    dev_cfg.queue_size = 16;

    spi_device_handle_t spi_device;
    ESP_ERROR_CHECK(spi_bus_add_device(SPI3_HOST, &dev_cfg, &spi_device));

    ESP_ERROR_CHECK(sx127x_create(spi_device, &sx127xDevice));

    ESP_ERROR_CHECK(sx127x_set_opmod(SX127x_MODE_SLEEP, SX127x_MODULATION_OOK,
                                     sx127xDevice));

    ESP_ERROR_CHECK(sx127x_set_opmod(SX127x_MODE_FSRX, SX127x_MODULATION_FSK,
                                     sx127xDevice));
    // enable temp monitoring
    ESP_ERROR_CHECK(sx127x_fsk_ook_set_temp_monitor(true, sx127xDevice));
    // a little bit longer for FSRX mode to kick off
    vTaskDelay(0.1 / portTICK_PERIOD_MS);
    // disable temp monitoring
    ESP_ERROR_CHECK(sx127x_fsk_ook_set_temp_monitor(false, sx127xDevice));
    ESP_ERROR_CHECK(sx127x_set_opmod(SX127x_MODE_SLEEP, SX127x_MODULATION_FSK,
                                     sx127xDevice));

    int8_t raw_temperature;
    ESP_ERROR_CHECK(
        sx127x_fsk_ook_get_raw_temperature(sx127xDevice, &raw_temperature));
    ESP_LOGI(TAG, "raw temperature: %d", raw_temperature);

    uint8_t registers[0x80];
    sx127x_dump_registers(registers, sx127xDevice);

    for (unsigned int i = 0; i < 0x80; i++) {
        ESP_LOGI(TAG, "%02x = %02x", i, registers[i]);
    }

    // ESP_ERROR_CHECK(sx127x_set_frequency(433920000, sx127xDevice));

    // ESP_ERROR_CHECK(
    //     sx127x_set_opmod(SX127x_MODE_STANDBY, SX127x_MODULATION_OOK,
    //     sx127xDevice));

    // ESP_ERROR_CHECK(sx127x_fsk_ook_set_continuous_mode(sx127xDevice));

    // ESP_ERROR_CHECK(sx127x_fsk_ook_rx_set_afc_auto(false, sx127xDevice));
    // ESP_ERROR_CHECK(sx127x_fsk_ook_rx_set_bandwidth(5000.0, sx127xDevice));

    // // ESP_ERROR_CHECK(sx127x_rx_set_lna_gain(SX127x_LNA_GAIN_AUTO, device));
    // ESP_ERROR_CHECK(sx127x_rx_set_lna_gain(SX127x_LNA_GAIN_G3,
    // sx127xDevice));

    // ESP_ERROR_CHECK(sx127x_fsk_ook_set_bitrate(2000.0, sx127xDevice));

    // // ESP_ERROR_CHECK(sx127x_ook_rx_set_peak_mode(SX127X_0_5_DB, 0x0C,
    // // SX127X_1_1_CHIP, device));
    // ESP_ERROR_CHECK(sx127x_ook_rx_set_fixed_mode(0x20, sx127xDevice));

    // // ESP_ERROR_CHECK(sx127x_ook_rx_set_fixed_mode(SX127X_6_0_DB, device));

    // ESP_ERROR_CHECK(sx127x_fsk_ook_rx_set_rssi_config(SX127X_8, 0,
    // sx127xDevice));

    // ESP_ERROR_CHECK(sx127x_ook_set_data_shaping(SX127X_OOK_SHAPING_NONE,
    //                                             SX127X_PA_RAMP_10,
    //                                             sx127xDevice));

    // ESP_ERROR_CHECK(
    //     sx127x_set_opmod(SX127x_MODE_RX_CONT, SX127x_MODULATION_OOK,
    //     sx127xDevice));
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
            i2c_master_write_byte(cmd, (address << 1) | WRITE_BIT,
                                  ACK_CHECK_EN);
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
        .atten = ADC_ATTEN_DB_11,
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
        .atten = ADC_ATTEN_DB_11,
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

    ESP_LOGI(TAG, "setup SPI3_HOST");
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_SPI3_MOSI,
        .miso_io_num = PIN_SPI3_MISO,
        .sclk_io_num = PIN_SPI3_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .data4_io_num = 0,
        .data5_io_num = 0,
        .data6_io_num = 0,
        .data7_io_num = 0,
        .max_transfer_sz = 200000,
        .flags = 0,
        .isr_cpu_id = (intr_cpu_id_t)0,
        .intr_flags = 0,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO));

    setupLora();

    ESP_LOGI(TAG, "Initialize I2C bus");
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = PIN_NUM_SDA,
        .scl_io_num = PIN_NUM_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master =
            {
                .clk_speed = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
            },
        .clk_flags = 0,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));

    scanI2CBus();

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = 0x3c,
        .control_phase_bytes = 1,
        .dc_bit_offset = 6,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(
        (esp_lcd_i2c_bus_handle_t)I2C_NUM_0, &io_config, &io_handle));

    esp_lcd_panel_handle_t p = display::createSSD1306Panel(io_handle);

    // setupJoystick();

    // static int adc_raw[2];
    // static int voltage[2];
    while (true) {
        // ESP_ERROR_CHECK(
        //     adc_oneshot_read(oneshot_handle, ADC_CHANNEL_4, &adc_raw[0]));
        // ESP_ERROR_CHECK(
        //     adc_oneshot_read(oneshot_handle, ADC_CHANNEL_5, &adc_raw[1]));
        // ESP_LOGI(TAG, "x=%d y=%d", adc_raw[0], adc_raw[1]);
        // if (calibration_handle) {
        //     // ESP_ERROR_CHECK(adc_cali_raw_to_voltage(calibration_handle,
        //     //                                         adc_raw[0],
        //     //                                         &voltage[0]));
        //     // ESP_ERROR_CHECK(adc_cali_raw_to_voltage(calibration_handle,
        //     //                                         adc_raw[1],
        //     //                                         &voltage[1]));
        //     ESP_LOGI(TAG, "x=%d mV y=%d mV", voltage[0], voltage[1]);
        // }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
