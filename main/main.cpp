#include <driver/gpio.h>
#include <nvs_flash.h>
#include <string.h>

#include <esp-display-ili9341.hpp>
#include <lvgl-touch-xpt2046.hpp>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp-display-backlight.hpp"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "esp_netif.h"
#include "esp_netif_sntp.h"
#include "gui/home-controller.hpp"
#include "http-server.hpp"
#include "lvgl-mvc/lvgl-mvc.hpp"
#include "lvgl-mvc/navigation.hpp"
#include "lvgl.h"
#include "model/energy-model.hpp"
#include "mqtt-client.hpp"
#include "neopixel.hpp"
#include "rtc_wdt.h"
#include "spiffs.hpp"
#include "time.h"
#include "weather-client.hpp"
#include "wifi.hpp"

#define TAG "main"

#define NUM_LEDS 1

extern bool lvgl_mvc_lock(uint32_t timeout_ms)
{
    return lvgl_port_lock(timeout_ms);
}

extern void lvgl_mvc_unlock(void)
{
    lvgl_port_unlock();
}

DisplayNavigationContoller displayNavigationContoller;
HomeViewController homeViewController(NULL);
EnergyModel energyModel;

void heap_caps_alloc_failed_hook(size_t requested_size, uint32_t caps,
                                 const char* function_name);

static Pixels* neoPixels = NULL;

/* LCD IO and panel */
static esp_lcd_panel_io_handle_t lcd_io = NULL;
static esp_lcd_panel_handle_t lcd_panel = NULL;
static lv_display_t* lvgl_disp = NULL;

#define LCD_H_RES 320
#define LCD_V_RES 240
#define LCD_DRAW_BUFF_HEIGHT 240
#define LCD_DRAW_BUFF_DOUBLE true

void startGuiContoller()
{
    lvgl_port_lock(0);

    initializeScreenTouchXpt2046();

    lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE),
                          lv_palette_main(LV_PALETTE_RED), true,
                          &lv_font_montserrat_12);

    displayNavigationContoller.setDisplay(lv_display_get_default());
    displayNavigationContoller.pushViewController(homeViewController);

    lv_timer_t *timer = lv_timer_create(
        [](lv_timer_t* timer) { readWeatherService(); }, 1000 * 10, NULL);
    lv_timer_ready(timer);

    lvgl_port_unlock();
}

static esp_err_t app_lvgl_init(void)
{
    /* Initialize LVGL */
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 2,  /* LVGL task priority */
        .task_stack = 8196,  /* LVGL task stack size */
        .task_affinity = -1, /* LVGL task pinned to core (-1 is no affinity) */
        .task_max_sleep_ms = 500, /* Maximum sleep in LVGL task */
        .timer_period_ms = 5,     /* LVGL timer tick period in ms */
    };
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG,
                        "LVGL port initialization failed");

    /* Add LCD screen */
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = lcd_io,
        .panel_handle = lcd_panel,
        .control_handle = NULL,
        .buffer_size = LCD_H_RES * LCD_V_RES,
        .double_buffer = LCD_DRAW_BUFF_DOUBLE,
        .trans_size = 0,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
        .rotation =
            {
                .swap_xy = true,
                .mirror_x = false,
                .mirror_y = false,
            },
        .color_format = LV_COLOR_FORMAT_RGB565,
        .flags =
            {
                .buff_dma = true,
                .buff_spiram = true,
                .sw_rotate = false,
                .swap_bytes = true,
                .full_refresh = true,
                .direct_mode = false,

            },
    };
    lvgl_disp = lvgl_port_add_disp(&disp_cfg);

    return ESP_OK;
}

extern "C" void app_main(void)
{
    heap_caps_register_failed_alloc_callback(heap_caps_alloc_failed_hook);

    /* Initialize NVS partition */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        /* NVS partition was truncated
         * and needs to be erased */
        ESP_ERROR_CHECK(nvs_flash_erase());

        /* Retry nvs_flash_init */
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_IRAM));

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    neoPixels = new Pixels(GPIO_NUM_48, NUM_LEDS, Pixels::StripType::SK68XXMINI,
                           Pixels::ColorOrder::GRB, 2.8);

    neoPixels->setPixel(
        0, {.red = 0x00, .green = 0x00, .blue = 0x00, .white = 0x00});
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
        .max_transfer_sz = 0,
        .flags = 0,
        .isr_cpu_id = (esp_intr_cpu_affinity_t)0,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg2, SPI_DMA_CH_AUTO));

    lcd_io = display::createIli9341SpiPanelIO(
        SPI2_HOST, (gpio_num_t)CONFIG_PRJ_PIN_ILI9341_CS,
        (gpio_num_t)CONFIG_PRJ_PIN_ILI9341_DC, 8, 8, 40 * 1000 * 1000);

    lcd_panel = display::createIli9341Panel(
        lcd_io, (gpio_num_t)CONFIG_PRJ_PIN_ILI9341_RES);

    ESP_ERROR_CHECK(app_lvgl_init());

    startGuiContoller();

    display::setupBacklightPin((gpio_num_t)CONFIG_PRJ_PIN_ILI9341_BK_LIGHT);
    display::setBacklight((gpio_num_t)CONFIG_PRJ_PIN_ILI9341_BK_LIGHT, 1);

    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_netif_sntp_init(&config);

    ESP_ERROR_CHECK(startSpiffs("/spiffs"));

    start_wifi(true);

    httpd_handle_t httpdHandle;
    ESP_ERROR_CHECK(startWebserver(&httpdHandle, "/spiffs"));

    mqtt_app_start();

    readWeatherService();

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void* lv_malloc_core(size_t size)
{
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA);
}

void* lv_realloc_core(void* p, size_t new_size)
{
    return heap_caps_realloc(p, new_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA);
}

void lv_free_core(void* data)
{
    heap_caps_free(data);
}

void lv_mem_init(void) {}

void lv_mem_deinit(void) {}

void heap_caps_alloc_failed_hook(size_t requested_size, uint32_t caps,
                                 const char* function_name)
{
    ESP_LOGE(TAG,
             "%s was called but failed to allocate %d bytes with 0x%lu "
             "capabilities. \n",
             function_name, requested_size, caps);
}
