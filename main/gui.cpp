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

extern sx127x *sx127xDevice;

#define TAG "gui"

#if CONFIG_PRJ_ST7798_SPI1_HOST == 1
#define ST9978_SPI SPI1_HOST
#elif CONFIG_PRJ_ST7798_SPI2_HOST == 1
#define ST9978_SPI SPI2_HOST
#elif CONFIG_PRJ_ST7798_SPI3_HOST == 1
#define ST9978_SPI SPI3_HOST
#endif

ESP_EVENT_DEFINE_BASE(GUI_EVENTS);

static RotaryEncoder *encoder;
static esp_lcd_panel_io_handle_t ssd1306_panel_io_handle;
static esp_lcd_panel_handle_t ssd1306_panel_handle;
static esp_lcd_panel_io_handle_t st7798_panel_io_handle;
static esp_lcd_panel_handle_t st7798_panel_handle;

static lv_group_t *g_group;
static lv_obj_t *screen1, *screen2;
static lv_obj_t *labelIpAddress;
static lv_obj_t *slider_label;
static lv_obj_t *slider;
static lv_obj_t *cb_power;
static int8_t lowPowerOptions[] = {-4, -3, -2, -1, 0,  1,  2,  3,  4, 5,
                                   6,  7,  8,  9,  10, 11, 12, 13, 14};
static int8_t highPowerOptions[] = {2,  3,  4,  5,  6,  7,  8,  9, 10,
                                    11, 12, 13, 14, 15, 16, 17, 20};
static bool highPower = false;
static uint8_t powerIndex = 0;

extern uint8_t test_img[];

void setIpAddressText(char *text)
{
    lcd::lvglLock(-1);

    lv_label_set_text(labelIpAddress, text);

    lcd::lvglUnlock();
}

lv_obj_t *infoScreen()
{
    lv_obj_t *label;

    lcd::lvglLock(-1);

    lv_obj_t *scr = lv_obj_create(NULL);

    label = lv_label_create(scr);
    lv_label_set_text(label, "IP Adress:");
    lv_obj_set_pos(label, 0, 0);

    labelIpAddress = lv_label_create(scr);
    lv_label_set_text(labelIpAddress, "-");
    lv_obj_set_pos(labelIpAddress, 80, 40);

    lcd::lvglUnlock();

    return scr;
}

static void update_power_settings()
{
    lv_state_t cbState = lv_obj_get_state(cb_power);

    if (!(cbState & LV_STATE_CHECKED)) {
        lv_slider_set_range(slider, 0, sizeof(lowPowerOptions) - 1);
        if (highPower) {
            highPower = false;
            powerIndex = 0;
            lv_slider_set_value(slider, powerIndex, LV_ANIM_OFF);
        }
    } else {
        lv_slider_set_range(slider, 0, sizeof(highPowerOptions) - 1);
        if (!highPower) {
            highPower = true;
            powerIndex = 0;
            lv_slider_set_value(slider, powerIndex, LV_ANIM_OFF);
        }
    }

    powerIndex = lv_slider_get_value(slider);

    int16_t power;
    if (!highPower) {
        power = lowPowerOptions[powerIndex];
        ESP_ERROR_CHECK(
            sx127x_tx_set_pa_config(SX127x_PA_PIN_RFO, power, sx127xDevice));
    } else {
        power = highPowerOptions[powerIndex];
        ESP_ERROR_CHECK(
            sx127x_tx_set_pa_config(SX127x_PA_PIN_BOOST, power, sx127xDevice));
    }

    char buf[8];
    snprintf(buf, sizeof(buf), "%d", (int16_t)power);
    lv_label_set_text(slider_label, buf);
}

static void event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (target == slider && code == LV_EVENT_VALUE_CHANGED) {
        update_power_settings();
    } else if (target == cb_power && code == LV_EVENT_VALUE_CHANGED) {
        update_power_settings();
    }
}

lv_obj_t *createLoraControllScreen()
{
    lcd::lvglLock(-1);

    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_pad_all(scr, 20, LV_STATE_DEFAULT);

    // cb_power = lv_checkbox_create(scr);
    cb_power = lv_switch_create(scr);
    // lv_checkbox_set_text(cb_power, "High power");
    // lv_obj_set_style_text_font(cb_power, &lv_font_montserrat_14, 0);
    // lv_obj_set_size(cb_power, 30, 20);
    lv_obj_add_event_cb(cb_power, event_handler, LV_EVENT_ALL, NULL);
    lv_group_add_obj(g_group, cb_power);

    lv_obj_t *label = lv_label_create(scr);
    lv_obj_align_to(label, cb_power, LV_ALIGN_OUT_RIGHT_TOP, 10, 0);
    lv_checkbox_set_text(label, "High power");

    slider = lv_slider_create(scr);
    lv_obj_set_size(slider, LV_PCT(100), 20);
    lv_obj_align_to(slider, cb_power, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    lv_obj_add_event_cb(slider, event_handler, LV_EVENT_ALL, NULL);
    lv_group_add_obj(g_group, slider);
    lv_obj_set_style_radius(slider, 0, 0);
    lv_obj_set_style_radius(slider, 0, LV_PART_INDICATOR);

    slider_label = lv_label_create(scr);
    lv_label_set_text(slider_label, "-");
    lv_obj_set_style_text_align(slider_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_size(slider_label, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_align_to(slider_label, slider, LV_ALIGN_TOP_LEFT, 0, 0);

    update_power_settings();

    lcd::lvglUnlock();

    return scr;
}

// Menlo, Monaco, 'Courier New', monospace

lv_obj_t *createTestScreen1()
{
    lcd::lvglLock(-1);

    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_pad_all(scr, 20, LV_STATE_DEFAULT);

    lv_obj_t *sw = lv_switch_create(scr);
    lv_group_add_obj(g_group, sw);
    lv_obj_set_pos(sw, 0, 0);

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
        // lv_style_set_line_color(&style, lv_palette_main(LV_PALETTE_RED));
        // lv_style_set_border_color(&style, lv_palette_main(LV_PALETTE_RED));
        lv_style_set_outline_color(&style, lv_palette_main(LV_PALETTE_RED));
        // lv_style_set_bg_color(&style, lv_palette_main(LV_PALETTE_RED));
        lv_style_set_outline_width(&style, 5);
    }

    if (lv_obj_check_type(obj, &lv_checkbox_class)) {
        lv_obj_add_style(obj, &style, LV_PART_INDICATOR | LV_STATE_EDITED);
    } else if (lv_obj_check_type(obj, &lv_slider_class)) {
        lv_obj_add_style(obj, &style, LV_PART_KNOB | LV_STATE_EDITED);
    }
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

    lv_disp_set_theme(disp1, &th_new1);
    th_new1.font_large = &lv_font_montserrat_20;

    st7798_panel_io_handle = display::createSt7789SpiPanelIO(
        ST9978_SPI, (gpio_num_t)CONFIG_PRJ_PIN_ST7789_CS,
        (gpio_num_t)CONFIG_PRJ_PIN_ST7789_DC, 8, 8, 20 * 1000 * 1000);

    st7798_panel_handle = display::createSt7789Panel(
        st7798_panel_io_handle, (gpio_num_t)CONFIG_PRJ_PIN_ST7789_RES);

    lv_disp_t *disp2 = lcd::registerDisplay(
        st7798_panel_io_handle, st7798_panel_handle, lcd::ST7789, 240, 240, 16);

    display::setupBacklightPin((gpio_num_t)CONFIG_PRJ_PIN_ST7789_BK_LIGHT);
    display::setBacklight((gpio_num_t)CONFIG_PRJ_PIN_ST7789_BK_LIGHT, 1);

    lv_theme_t *th2 = lv_theme_default_init(
        NULL, lv_palette_main(LV_PALETTE_BLUE),
        lv_palette_main(LV_PALETTE_CYAN), true, &lv_font_montserrat_14);

    static lv_theme_t th_new2;
    lv_theme_set_parent(&th_new2, th2);

    lv_theme_set_apply_cb(&th_new2, custom_apply_cb);

    lv_disp_set_theme(disp2, &th_new2);
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

    encoder = new RotaryEncoder((gpio_num_t)CONFIG_PRJ_PIN_ROTARY_CLK,
                                (gpio_num_t)CONFIG_PRJ_PIN_ROTARY_DATA,
                                (gpio_num_t)CONFIG_PRJ_PIN_ROTARY_SWITCH);
    lcd::createLvglRotaryInputDev(*encoder);

    lv_indev_set_group(lcd::rotary_indev, g_group);

    lv_disp_set_default(disp1);
    screen1 = infoScreen();
    lv_scr_load(screen1);

    lv_disp_set_default(disp2);

    screen2 = createLoraControllScreen();
    lv_scr_load(screen2);
    createTestScreen1();

    return ESP_OK;
}
