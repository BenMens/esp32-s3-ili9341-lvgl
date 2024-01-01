#include "gui.hpp"

#include <stdio.h>
#include <sx127x.h>

#include "driver/i2c.h"
#include "esp-display.hpp"
#include "esp_event.h"
#include "esp_log.h"
#include "lvgl-display.hpp"
#include "rotary_encoder.hpp"
#include "sdkconfig.h"

#define TAG "main"

ESP_EVENT_DEFINE_BASE(GUI_EVENTS);

static RotaryEncoder *encoder;
static esp_lcd_panel_io_handle_t ssd1306_panel_io_handle;
static esp_lcd_panel_handle_t ssd1306_panel_handle;
static esp_lcd_panel_io_handle_t st7798_panel_io_handle;
static esp_lcd_panel_handle_t st7798_panel_handle;

static lv_group_t *g_group;
static lv_obj_t *screen1, *screen2;
static lv_obj_t *labelIpAddress;
static lv_obj_t *cocoProtocol;
static lv_obj_t *cocoAdres;
static lv_obj_t *cocoUnit;
static lv_obj_t *cocoState;

extern uint8_t test_img[];

void setIpAddressText(char *text)
{
    lcd::lvglLock(-1);

    lv_label_set_text(labelIpAddress, text);

    lcd::lvglUnlock();
}

void btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED) {
        static uint8_t cnt = 0;
        cnt++;

        lv_obj_t *label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Button: %d", cnt);
    }
}

lv_obj_t *createScreen1()
{
    lv_obj_t *label;

    lcd::lvglLock(-1);

    lv_obj_t *scr = lv_obj_create(NULL);

    label = lv_label_create(scr);
    lv_label_set_text(label, "IP address:");
    lv_obj_set_pos(label, 0, 0);

    label = lv_label_create(scr);
    lv_label_set_text(label, "test1");
    lv_obj_set_pos(label, 80, 40);

    lcd::lvglUnlock();

    return scr;
}

lv_obj_t *createScreen2()
{
    lv_obj_t *label;

    lcd::lvglLock(-1);

    lv_obj_t *scr = lv_obj_create(NULL);

    label = lv_label_create(scr);
    lv_label_set_text(label, "QQ");
    lv_obj_set_pos(label, 0, 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_palette_main(LV_PALETTE_RED), 0);

    label = lv_label_create(scr);
    lv_label_set_text(label, "JJ");
    lv_obj_set_pos(label, 80, 40);

    lcd::lvglUnlock();

    return scr;
}

static void custom_apply_cb(struct _lv_theme_t *theme, lv_obj_t *obj)
{
    static bool initialized = false;
    static lv_style_t style;

    if (!initialized) {
        initialized = true;
        lv_style_init(&style);
        lv_style_set_outline_color(&style, lv_palette_main(LV_PALETTE_RED));
        lv_style_set_outline_width(&style, 5);
    }

    lv_obj_add_style(obj, &style, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
}

esp_err_t guiInit()
{
    lcd::startLvgl();

    ssd1306_panel_io_handle = display::createSsd1306I2CPanelIO(
        (esp_lcd_i2c_bus_handle_t)I2C_NUM_0, 0x3c);

    ssd1306_panel_handle = display::createSsd1306Panel(ssd1306_panel_io_handle);

    esp_lcd_panel_mirror(ssd1306_panel_handle, true, true);

    lv_disp_t *disp1 =
        lcd::registerDisplay(ssd1306_panel_io_handle, ssd1306_panel_handle,
                             lcd::SSD1306, 128, 64, 24);

    lv_theme_t *th1 = lv_theme_mono_init(disp1, false, &lv_font_montserrat_12);

    static lv_theme_t th_new1;
    lv_theme_set_parent(&th_new1, th1);

    lv_theme_set_apply_cb(&th_new1, custom_apply_cb);

    lv_disp_set_theme(disp1, &th_new1);
    th_new1.font_large = &lv_font_montserrat_20;


    st7798_panel_io_handle = display::createSt7789SpiPanelIO(
        SPI2_HOST, (gpio_num_t)CONFIG_PRJ_PIN_ST7789_CS,
        (gpio_num_t)CONFIG_PRJ_PIN_ST7789_DC, 8, 8, 20 * 1000 * 1000);

    st7798_panel_handle = display::createSt7789Panel(
        st7798_panel_io_handle, (gpio_num_t)CONFIG_PRJ_PIN_ST7789_RES);

    lv_disp_t *disp2 = lcd::registerDisplay(
        st7798_panel_io_handle, st7798_panel_handle, lcd::ST7789, 240, 240, 16);

    display::setupBacklightPin((gpio_num_t)CONFIG_PRJ_PIN_ST7789_BK_LIGHT);
    display::setBacklight((gpio_num_t)CONFIG_PRJ_PIN_ST7789_BK_LIGHT, 1);

    lv_theme_t *th2 = lv_theme_default_init(
        NULL, lv_palette_main(LV_PALETTE_BLUE),
        lv_palette_main(LV_PALETTE_CYAN), false, &lv_font_montserrat_14);

    static lv_theme_t th_new2;
    lv_theme_set_parent(&th_new2, th2);

    lv_theme_set_apply_cb(&th_new2, custom_apply_cb);

    lv_disp_set_theme(disp2, &th_new2);
    th_new2.font_large = &lv_font_montserrat_20;

    g_group = lv_group_create();
    lv_group_set_default(g_group);

    lv_group_set_wrap(g_group, false);
    lv_group_set_focus_cb(g_group, [](lv_group_t *g) {
        lv_obj_t *focused_object = lv_group_get_focused(g);
        lv_obj_t *focused_screen = lv_obj_get_screen(focused_object);

        if (focused_screen != lv_disp_get_scr_act(NULL)) {
            lv_scr_load_anim(focused_screen, LV_SCR_LOAD_ANIM_OVER_BOTTOM, 200,
                             0, false);
        }
    });

    // encoder = new RotaryEncoder(GPIO_NUM_32, GPIO_NUM_33, GPIO_NUM_27);
    // lcd::createLvglRotaryInputDev(*encoder);
    // lv_indev_set_group(lcd::rotary_indev, g_group);

    lv_disp_set_default(disp1);
    screen1 = createScreen1();
    lv_scr_load(screen1);


    lv_disp_set_default(disp2);
    screen2 = createScreen2();
    lv_scr_load(screen2);
    lv_group_add_obj(g_group, screen2);


    return ESP_OK;
}
