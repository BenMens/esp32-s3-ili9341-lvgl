#include "weather-controller.hpp"

#include <stdio.h>

#include <cstring>

#include "../model/weather-model.hpp"

extern WeatherModel weatherModel;

WeatherViewController::WeatherViewController(
    ViewController *parentViewController)
    : ViewController(parentViewController), backButtonViewController(this)
{
    for (int i = 0; i < NUM_HOUR_VIEW_CONTROLLERS; i++) {
        weatherHourViewControllers[i] = new WeatherHourViewController(this, i);
    }
}

WeatherViewController::~WeatherViewController()
{
    for (int i = 0; i < NUM_HOUR_VIEW_CONTROLLERS; i++) {
        delete weatherHourViewControllers[i];
    }
}

lv_obj_t *WeatherViewController::createView(lv_obj_t *parent)
{
    lv_obj_t *view = lv_obj_create(parent);
    lv_obj_set_size(view, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(view, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(view, 0, 0);
    // lv_obj_set_style_pad_gap(view, 0, 0);

    lv_obj_t *topRow = lv_obj_create(view);
    lv_obj_set_size(topRow, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(topRow, LV_OPA_0, 0);
    lv_obj_set_style_pad_all(topRow, 0, 0);
    lv_obj_set_style_border_width(topRow, 0, 0);
    lv_obj_set_style_radius(topRow, 0, 0);

    lv_obj_t *bottomRow = lv_obj_create(view);
    lv_obj_set_width(bottomRow, lv_pct(100));
    lv_obj_set_style_bg_opa(bottomRow, LV_OPA_0, 0);
    lv_obj_set_style_pad_all(bottomRow, 0, 0);
    lv_obj_set_flex_grow(bottomRow, 1);
    lv_obj_set_style_border_width(bottomRow, 0, 0);
    lv_obj_set_style_radius(bottomRow, 0, 0);
    lv_obj_set_flex_flow(bottomRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(bottomRow, 4, 0);

    for (int i = 0; i < NUM_HOUR_VIEW_CONTROLLERS; i++) {
        weatherHourViewControllers[i]->getViewAttachedToParent(bottomRow);
    }

    backButtonViewController.getViewAttachedToParent(topRow);

    return view;
}

void WeatherViewController::onPushed()
{
    for (int i = 0; i < NUM_HOUR_VIEW_CONTROLLERS; i++) {
        weatherHourViewControllers[i]->onPushed();
    }
}

void WeatherViewController::onPopped()
{
    for (int i = 0; i < NUM_HOUR_VIEW_CONTROLLERS; i++) {
        weatherHourViewControllers[i]->onPopped();
    }
}

void WeatherViewController::update()
{
    if (!viewValid()) return;

    if (lvgl_mvc_lock(0)) {
        for (int i = 0; i < NUM_HOUR_VIEW_CONTROLLERS; i++) {
            weatherHourViewControllers[i]->update();
        }
        lvgl_mvc_unlock();
    }
}
