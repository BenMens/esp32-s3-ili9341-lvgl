#include "clock-controller.hpp"

#include <stdio.h>
#include <string.h>
#include <time.h>

static const char *hour_ticks[] = {"12", "1", "2", "3",  "4",  "5", "6",
                                   "7",  "8", "9", "10", "11", NULL};

ClockViewController::ClockViewController(ViewController *parentViewController,
                                         TemperatureModel &temperatureModel)
    : ViewController(parentViewController),
      temperatureModel(temperatureModel),
      backButtonViewController(this)
{
}

lv_obj_t *ClockViewController::createView(lv_obj_t *parent)
{
    static lv_style_t minor_ticks_style;
    static lv_style_t main_line_style;
    static lv_style_t indicator_style;

    lv_obj_t *view = lv_obj_create(parent);
    lv_obj_set_size(view, lv_pct(100), lv_pct(100));

    clock = lv_scale_create(view);
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
    lv_obj_set_style_line_width(second_hand, 3, 0);
    lv_obj_set_style_line_rounded(second_hand, true, 0);
    lv_obj_set_style_line_color(second_hand, lv_color_make(0xff, 0x00, 0x00),
                                0);

    minute_hand = lv_line_create(clock);
    lv_line_set_points_mutable(minute_hand, minute_hand_points, 2);
    lv_obj_set_style_line_width(minute_hand, 5, 0);
    lv_obj_set_style_line_rounded(minute_hand, true, 0);
    lv_obj_set_style_line_color(minute_hand, lv_color_black(), 0);

    hour_hand = lv_line_create(clock);
    lv_obj_set_style_line_width(hour_hand, 7, 0);
    lv_obj_set_style_line_rounded(hour_hand, true, 0);
    lv_obj_set_style_line_color(hour_hand, lv_color_black(), 0);

    temperatureLabel = lv_label_create(view);
    lv_obj_set_pos(temperatureLabel, 10, 10);
    lv_label_set_text(temperatureLabel, "-");

    humidityLabel = lv_label_create(view);
    lv_obj_set_pos(humidityLabel, 10, 30);
    lv_label_set_text(humidityLabel, "-");

    backButtonViewController.getViewAttachedToParent(view);

    return view;
}

void ClockViewController::onDidAppear()
{
    temperatureModelRegistration = temperatureModel.events.addHandler(
        TemperatureModelEvents::MEASUREMENT_CHANGED, nullptr,
        [&](TemperatureModel &source, TemperatureModelEvents event,
            TemperatureModelEventData *eventData,
            void *userData) { update(); });

    if (lvgl_mvc_lock(0)) {
        timer = lv_timer_create(
            [](lv_timer_t *timer) {
                ClockViewController *controller =
                    (ClockViewController *)lv_timer_get_user_data(timer);
                controller->update();
            },
            1000, this);

        lvgl_mvc_unlock();
    }
}

void ClockViewController::onWillDisappear()
{
    if (lvgl_mvc_lock(0)) {
        lv_timer_delete(timer);

        lvgl_mvc_unlock();
    }

    temperatureModel.events.removeHandler(temperatureModelRegistration);
}

void ClockViewController::update()
{
    if (!viewValid()) return;

    char tmp[20];

    snprintf(tmp, sizeof(tmp), "%.2f °C", temperatureModel.getTemperature());
    lv_label_set_text(temperatureLabel, tmp);
    
    snprintf(tmp, sizeof(tmp), "%.2f %%", temperatureModel.getHumidity());
    lv_label_set_text(humidityLabel, tmp);

    if (lvgl_mvc_lock(0)) {
        time_t now;
        time(&now);

        struct tm *t = localtime(&now);

        lv_scale_set_line_needle_value(clock, second_hand, 60, t->tm_sec);
        lv_scale_set_line_needle_value(clock, minute_hand, 60, t->tm_min);
        lv_scale_set_line_needle_value(clock, hour_hand, 40,
                                       t->tm_hour % 12 * 5 + (t->tm_min / 12));

        lvgl_mvc_unlock();
    }
}
