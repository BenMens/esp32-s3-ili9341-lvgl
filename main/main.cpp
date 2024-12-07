#include <string.h>

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "driver/uart.h"
#include "esp-display-backlight.hpp"
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

static Pixels *neoPixels = NULL;

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_IRAM));

    neoPixels = new Pixels(GPIO_NUM_48, NUM_LEDS, Pixels::StripType::SK68XXMINI,
                           Pixels::ColorOrder::GRB, 2.8);

    neoPixels->setPixel(0, {
                               .red = 0x00,
                               .green = 0x00,
                               .blue = 0x00,
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
            320 * 80 *
            sizeof(uint16_t),  // TODO parameterize this based on LCD settings
        .flags = 0,
        .isr_cpu_id = (esp_intr_cpu_affinity_t)0,
        .intr_flags = 0,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg2, SPI_DMA_CH_AUTO));

    guiInit();

    vTaskDelay(2000 / portTICK_PERIOD_MS);
}
