#include "weather-hour-controller.hpp"

#include <stdio.h>

#include <cstring>

#include "esp_log.h"

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

    timeControll = lv_label_create(view);
    lv_obj_align(timeControll, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_font(timeControll, &lv_font_montserrat_20, 0);

    temperatureControll = lv_label_create(view);
    lv_obj_align_to(temperatureControll, timeControll, LV_ALIGN_OUT_BOTTOM_LEFT,
                    0, 10);
    lv_obj_set_style_text_font(temperatureControll, &lv_font_montserrat_20, 0);

    iconControll = lv_image_create(view);
    lv_obj_align(iconControll, LV_ALIGN_TOP_RIGHT, 0, -20);

    return view;
}

void WeatherHourViewController::onPushed()
{
    ESP_LOGD(TAG, "Adding event subscription for WeatherHourViewController(%d)",
             forecastIndex);
    eventSubscription = weatherModel.events.addHandler(
        WeatherModelEvents::FORECAST_CHANGED, NULL,
        [&](WeatherModel &source, WeatherModelEvents event,
            WeatherModelEventData *eventData, void *userData) {
            if (eventData->index == forecastIndex) {
                update();
            }
        });
}

void WeatherHourViewController::onPopped()
{
    ESP_LOGD(TAG,
             "Removing event subscription for WeatherHourViewController(%d)",
             forecastIndex);
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

        lvgl_mvc_unlock();
    }
}
