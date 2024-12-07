#include "gui.hpp"

#include <stdio.h>

#include "driver/i2c.h"
#include "esp-display-backlight.hpp"
#include "esp-display-ili9341.hpp"
#include "esp_event.h"
#include "esp_log.h"
#include "lvgl-display.hpp"
#include "sdkconfig.h"

#define TAG "gui"

#define ILI9341_SPI SPI2_HOST

ESP_EVENT_DEFINE_BASE(GUI_EVENTS);

static esp_lcd_panel_io_handle_t ili9341_panel_io_handle;
static esp_lcd_panel_handle_t ili9341_panel_handle;

static lv_group_t *g_group;
static lv_obj_t *screen1;
static lv_obj_t *labelIpAddress;

extern uint8_t test_img[];

void setIpAddressText(char *text)
{
    lcd::lvglLock(-1);

    lv_label_set_text(labelIpAddress, text);

    lcd::lvglUnlock();
}

lv_obj_t *createScreen1()
{
    lv_obj_t *label;

    lcd::lvglLock(-1);

    lv_obj_t *scr = lv_obj_create(NULL);

    label = lv_label_create(scr);
    lv_label_set_text(label, "IP Adress:");
    lv_obj_set_pos(label, 0, 0);
    lv_obj_set_style_text_color(label, LV_COLOR_MAKE(0xE5, 0x00, 0x00), NULL);

    labelIpAddress = lv_label_create(scr);
    lv_label_set_text(labelIpAddress, "-");
    lv_obj_set_pos(labelIpAddress, 80, 40);

    lcd::lvglUnlock();

    return scr;
}

esp_err_t guiInit()
{
    lcd::startLvgl();

    ili9341_panel_io_handle = display::createIli9341SpiPanelIO(
        ILI9341_SPI, (gpio_num_t)CONFIG_PRJ_PIN_ILI9341_CS,
        (gpio_num_t)CONFIG_PRJ_PIN_ILI9341_DC, 8, 8, 40 * 1000 * 1000);

    ili9341_panel_handle = display::createIli9341Panel(
        ili9341_panel_io_handle, (gpio_num_t)CONFIG_PRJ_PIN_ILI9341_RES);

    lv_disp_t *disp1 =
        lcd::registerDisplay(ili9341_panel_io_handle, ili9341_panel_handle,
                             lcd::ILI9341, 320, 240, 16);

    lv_theme_t *th2 = lv_theme_default_init(
        NULL, lv_palette_main(LV_PALETTE_RED),
        lv_palette_main(LV_PALETTE_YELLOW), false, &lv_font_montserrat_12);

    static lv_theme_t th_new2;
    lv_theme_set_parent(&th_new2, th2);

    lv_disp_set_theme(disp1, &th_new2);
    th_new2.font_small = &lv_font_montserrat_20;
    th_new2.font_normal = &lv_font_montserrat_20;
    th_new2.font_large = &lv_font_montserrat_20;

    g_group = lv_group_create();

    lv_group_set_wrap(g_group, false);
    lv_group_set_focus_cb(g_group, [](lv_group_t *g) {
        lv_obj_t *focused_object = lv_group_get_focused(g);
        lv_obj_t *focused_screen = lv_obj_get_screen(focused_object);

        if (focused_screen !=
            lv_disp_get_scr_act(lv_obj_get_disp(focused_object))) {
            lv_scr_load_anim(focused_screen, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0,
                             false);
        }
    });

    lv_disp_set_default(disp1);

    screen1 = createScreen1();

    lv_scr_load(screen1);

    display::setupBacklightPin((gpio_num_t)CONFIG_PRJ_PIN_ILI9341_BK_LIGHT);
    display::setBacklight((gpio_num_t)CONFIG_PRJ_PIN_ILI9341_BK_LIGHT, 1);

    return ESP_OK;
}
