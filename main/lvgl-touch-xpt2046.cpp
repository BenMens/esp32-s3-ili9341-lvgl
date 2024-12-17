#include "lvgl-touch-xpt2046.hpp"

#include <esp_lcd_touch_xpt2046.h>
#include <esp_log.h>
#include <lvgl.h>
#include <sdkconfig.h>

#include "driver/spi_master.h"

#ifdef CONFIG_TOUCH_SPI2_HOST
#define TOUCH_SPI_HOST SPI2_HOST
#elif CONFIG_TOUCH_SPI3_HOST
#define TOUCH_SPI_HOST SPI3_HOST
#endif

#define TAG "lvgl-touch-xpt2046"

void initializeScreenTouchXpt2046()
{
    static lv_indev_t* indev_drv;
    static esp_lcd_touch_handle_t tp = NULL;

    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_spi_config_t tp_io_config =
        ESP_LCD_TOUCH_IO_SPI_XPT2046_CONFIG((gpio_num_t)CONFIG_TOUCH_CS_PIN);

    esp_lcd_touch_config_t tp_cfg = {
        .x_max = 320,
        .y_max = 240,
        .rst_gpio_num = (gpio_num_t)CONFIG_TOUCH_RESET_PIN,
        .int_gpio_num = (gpio_num_t)CONFIG_TOUCH_IRQ_PIN,
        .levels =
            {
                .reset = 0,
                .interrupt = 0,
            },
        .flags =
            {
                .swap_xy = 0,
                .mirror_x = 0,
                .mirror_y = 1,
            },
        .process_coordinates = NULL,
        .interrupt_callback = NULL,
        .user_data = NULL,
        .driver_data = NULL,
    };

    ESP_ERROR_CHECK(
        esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)TOUCH_SPI_HOST,
                                 &tp_io_config, &tp_io_handle));

    ESP_LOGI(TAG, "Initialize touch controller XPT2046");
    ESP_ERROR_CHECK(esp_lcd_touch_new_spi_xpt2046(tp_io_handle, &tp_cfg, &tp));

    indev_drv = lv_indev_create();
    lv_indev_set_type(indev_drv, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev_drv, [](lv_indev_t* drv, lv_indev_data_t* data) {
        uint16_t x[1];
        uint16_t y[1];
        uint16_t strength[1];
        uint8_t count = 0;

        // lcd::lvglLock(100);

        // Update touch point data.
        ESP_ERROR_CHECK(esp_lcd_touch_read_data(tp));

        data->state = LV_INDEV_STATE_REL;

        if (esp_lcd_touch_get_coordinates(tp, x, y, strength, &count, 1)) {
            data->point.x = x[0];
            data->point.y = y[0];
            data->state = LV_INDEV_STATE_PR;
        }

        data->continue_reading = false;

        // lcd::lvglUnlock();
    });
}
