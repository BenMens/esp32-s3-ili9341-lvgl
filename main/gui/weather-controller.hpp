#pragma once

#include "../lvgl-mvc/view-controller.hpp"
#include "back-button-controller.hpp"
#include "weather-hour-controller.hpp"

#define NUM_HOUR_VIEW_CONTROLLERS 12

class WeatherViewController : public ViewController
{
   protected:
    BackButtonViewController backButtonViewController;
    WeatherHourViewController *weatherHourViewControllers[NUM_HOUR_VIEW_CONTROLLERS];

    lv_obj_t *createView(lv_obj_t *parent);

   public:
    WeatherViewController(ViewController *parentViewController);
    ~WeatherViewController();

    void onPushed();
    void onPopped();
    void update();
};
