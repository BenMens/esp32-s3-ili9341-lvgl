#pragma once

#include "../lvgl-mvc/view-controller.hpp"
#include "back-button-controller.hpp"
#include "weather-hour-controller.hpp"

class WeatherViewController : public ViewController
{
   protected:
    BackButtonViewController backButtonViewController;
    WeatherHourViewController *weatherHourViewControllers[12];

    lv_obj_t *createView(lv_obj_t *parent);

   public:
    WeatherViewController(ViewController *parentViewController);
    ~WeatherViewController();

    void update();
};
