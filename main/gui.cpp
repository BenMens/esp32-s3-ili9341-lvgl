#include "gui.hpp"

#include <stdio.h>
#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "sdkconfig.h"

#define TAG "gui"

ESP_EVENT_DEFINE_BASE(GUI_EVENTS);

static lv_point_precise_t minute_hand_points[2];
static lv_point_precise_t second_hand_points[2];
static lv_style_t minor_ticks_style;
static lv_style_t main_line_style;
static lv_style_t indicator_style;

static const char *hour_ticks[] = {"12", "1", "2", "3",  "4",  "5", "6",
                                   "7",  "8", "9", "10", "11", NULL};

lv_obj_t *provisioning_qr;
lv_obj_t *hour_hand;
lv_obj_t *minute_hand;
lv_obj_t *second_hand;
lv_obj_t *clock;

lv_obj_t *createTabClock(lv_obj_t *tabview)
{
    lv_obj_t *tab = lv_tabview_add_tab(tabview, "Clock");

    clock = lv_scale_create(tab);

    lv_obj_set_size(clock, 170, 170);

    lv_scale_set_mode(clock, LV_SCALE_MODE_ROUND_INNER);
    lv_obj_set_style_radius(clock, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(clock, true, 0);
    lv_obj_center(clock);

    lv_scale_set_label_show(clock, true);

    lv_scale_set_total_tick_count(clock, 61);
    lv_scale_set_major_tick_every(clock, 5);

    lv_scale_set_text_src(clock, hour_ticks);

    lv_style_init(&indicator_style);

    /* Label style properties */
    lv_style_set_text_font(&indicator_style, LV_FONT_DEFAULT);
    lv_style_set_text_color(&indicator_style, lv_color_make(0x00, 0x00, 0x00));
    lv_obj_set_style_pad_all(clock, 0, 0);

    /* Major tick properties */
    lv_style_set_line_color(&indicator_style, lv_color_make(0x00, 0x00, 0x00));
    lv_style_set_length(&indicator_style, 8);     /* tick length */
    lv_style_set_line_width(&indicator_style, 3); /* tick width */
    lv_obj_add_style(clock, &indicator_style, LV_PART_INDICATOR);

    /* Minor tick properties */
    lv_style_init(&minor_ticks_style);
    lv_style_set_line_color(&minor_ticks_style,
                            lv_color_make(0x00, 0x00, 0x00));
    lv_style_set_length(&minor_ticks_style, 2);     /* tick length */
    lv_style_set_line_width(&minor_ticks_style, 1); /* tick width */
    lv_obj_add_style(clock, &minor_ticks_style, LV_PART_ITEMS);

    /* Main line properties */
    lv_style_init(&main_line_style);
    lv_obj_set_style_bg_opa(clock, LV_OPA_100, 0);
    lv_style_set_arc_color(&main_line_style, lv_color_black());
    lv_style_set_arc_width(&main_line_style, 0);
    lv_obj_add_style(clock, &main_line_style, LV_PART_MAIN);

    lv_scale_set_range(clock, 0, 60);

    lv_scale_set_angle_range(clock, 360);
    lv_scale_set_rotation(clock, 270);

    second_hand = lv_line_create(clock);
    lv_line_set_points_mutable(second_hand, second_hand_points, 2);
    lv_obj_set_style_line_width(second_hand, 1, 0);
    lv_obj_set_style_line_rounded(second_hand, true, 0);
    lv_obj_set_style_line_color(second_hand, lv_color_black(), 0);

    minute_hand = lv_line_create(clock);
    lv_line_set_points_mutable(minute_hand, minute_hand_points, 2);
    lv_obj_set_style_line_width(minute_hand, 3, 0);
    lv_obj_set_style_line_rounded(minute_hand, true, 0);
    lv_obj_set_style_line_color(minute_hand, lv_color_black(), 0);

    hour_hand = lv_line_create(clock);
    lv_obj_set_style_line_width(hour_hand, 5, 0);
    lv_obj_set_style_line_rounded(hour_hand, true, 0);
    lv_obj_set_style_line_color(hour_hand, lv_color_black(), 0);

    return tab;
}

lv_obj_t *createTabLights(lv_obj_t *tabview)
{
    lv_obj_t *tab = lv_tabview_add_tab(tabview, "Lights");

    lv_obj_t *label = lv_label_create(tab);
    lv_label_set_text(label, "Lights");

    return tab;
}

lv_obj_t *createTabNetwork(lv_obj_t *tabview)
{
    lv_obj_t *tab = lv_tabview_add_tab(tabview, "Network");

    lv_color_t bg_color = lv_palette_lighten(LV_PALETTE_LIGHT_BLUE, 5);
    lv_color_t fg_color = lv_palette_darken(LV_PALETTE_BLUE, 4);

    provisioning_qr = lv_qrcode_create(tab);
    lv_qrcode_set_size(provisioning_qr, 200);
    lv_qrcode_set_dark_color(provisioning_qr, fg_color);
    lv_qrcode_set_light_color(provisioning_qr, bg_color);

    /*Set data*/
    const char *data = "https://lvgl.io";
    lv_qrcode_update(provisioning_qr, data, strlen(data));
    lv_obj_center(provisioning_qr);

    /*Add a border with bg_color*/
    lv_obj_set_style_border_color(provisioning_qr, bg_color, 0);
    lv_obj_set_style_border_width(provisioning_qr, 5, 0);

    return tab;
}

void createGui()
{
    lv_theme_t *theme = lv_theme_default_init(
        NULL, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
        true, &lv_font_montserrat_12);

    lv_display_set_theme(NULL, theme);

    lv_obj_t *tabview = lv_tabview_create(lv_screen_active());
    lv_tabview_set_tab_bar_position(tabview, LV_DIR_LEFT);
    lv_tabview_set_tab_bar_size(tabview, 80);

    createTabClock(tabview);
    createTabLights(tabview);
    createTabNetwork(tabview);
}
