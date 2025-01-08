#include "weather-hour-controller.hpp"

#include <stdio.h>

#include <cstring>

#define TAG "weather-hour-controller"

extern WeatherModel weatherModel;

WeatherHourViewController::WeatherHourViewController(
    ViewController *parentViewController, int forecastIndex)
    : ViewController(parentViewController), forecastIndex(forecastIndex)
{
}

lv_obj_t *WeatherHourViewController::createView(lv_obj_t *parent)
{
    lv_obj_t *view = lv_obj_create(parent);
    lv_obj_set_size(view, 158, lv_pct(100));
    lv_obj_set_style_pad_all(view, 5, 0);
    lv_obj_set_style_border_width(view, 1, 0);
    lv_obj_set_style_border_color(view, lv_color_make(0x80, 0x80, 0x80), 0);
    lv_obj_set_scrollbar_mode(view, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(view, LV_OBJ_FLAG_SCROLLABLE);

    timeControll = lv_label_create(view);
    lv_obj_align(timeControll, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_font(timeControll, &lv_font_montserrat_20, 0);

    temperatureControll = lv_label_create(view);
    lv_obj_align_to(temperatureControll, timeControll, LV_ALIGN_OUT_BOTTOM_LEFT,
                    0, 10);
    lv_obj_set_style_text_font(temperatureControll, &lv_font_montserrat_20, 0);

    iconControll = lv_image_create(view);
    lv_obj_align(iconControll, LV_ALIGN_TOP_RIGHT, 20, -20);

    lv_obj_t *windIcon = lv_image_create(view);
    lv_obj_align(windIcon, LV_ALIGN_LEFT_MID, -20, 0);
    lv_image_set_src(windIcon, "A:iconen/wind.png");

    windControll = lv_label_create(view);
    lv_obj_align(windControll, LV_ALIGN_LEFT_MID, 62, -7);
    lv_obj_set_style_text_font(windControll, &lv_font_montserrat_20, 0);

    windDirControll = lv_label_create(view);
    lv_obj_align(windDirControll, LV_ALIGN_LEFT_MID, 62, 13);
    lv_obj_set_style_text_font(windDirControll, &lv_font_montserrat_20, 0);

    rainControll = lv_label_create(view);
    lv_obj_align(rainControll, LV_ALIGN_BOTTOM_LEFT, 0, -20);
    lv_obj_set_style_text_font(rainControll, &lv_font_montserrat_20, 0);

    return view;
}

void WeatherHourViewController::onDidAppear()
{
    eventSubscription = weatherModel.events.addHandler(
        WeatherModelEvents::FORECAST_CHANGED, NULL,
        [&](WeatherModel &source, WeatherModelEvents event,
            WeatherModelEventData *eventData, void *userData) {
            if (eventData->index == forecastIndex) {
                update();
            }
        });
}

void WeatherHourViewController::onWillDisappear()
{
    weatherModel.events.removeHandler(eventSubscription);
    eventSubscription = NULL;
}

void WeatherHourViewController::update()
{
    if (!viewValid()) return;
    if (forecastIndex < 0) return;

    if (lvgl_mvc_lock(0)) {
        char strBuf[40];

        const ForecastHour &forecast =
            weatherModel.getForecasthour(forecastIndex);

        lv_label_set_text(timeControll, forecast.time);

        snprintf(strBuf, sizeof(strBuf), "%.1f °C", forecast.temperature);
        lv_label_set_text(temperatureControll, strBuf);

        snprintf(strBuf, sizeof(strBuf), "A:iconen/%s.png", forecast.icon);
        lv_image_set_src(iconControll, strBuf);

        snprintf(strBuf, sizeof(strBuf), "%d bft", forecast.windSpeed);
        lv_label_set_text(windControll, strBuf);

        lv_label_set_text(windDirControll, forecast.windDir);

        snprintf(strBuf, sizeof(strBuf), "%.1f mm", forecast.rain);
        lv_label_set_text(rainControll, strBuf);

        lvgl_mvc_unlock();
    }
}
