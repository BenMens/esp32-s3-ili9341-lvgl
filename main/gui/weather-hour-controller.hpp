#pragma once

#include "../lvgl-mvc/events.hpp"
#include "../lvgl-mvc/view-controller.hpp"
#include "../model/weather-model.hpp"

class WeatherHourViewController : public ViewController
{
   protected:
    EventSubscription eventSubscription;
    WeatherModel &weatherModel;

    lv_obj_t *createView(lv_obj_t *parent);
    lv_obj_t *temperatureControll;
    lv_obj_t *iconControll;
    lv_obj_t *timeControll;
    lv_obj_t *windControll;
    lv_obj_t *rainControll;
    lv_obj_t *windDirControll;
    int forecastIndex;

   public:
    WeatherHourViewController(ViewController *parentViewController,
                              int forecastIndex, WeatherModel &weatherModel);

    void update();

    void onDidAppear();
    void onWillDisappear();
};
