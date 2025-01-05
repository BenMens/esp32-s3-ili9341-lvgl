#pragma once

#include "../lvgl-mvc/view-controller.hpp"
#include "../model/weather-model.hpp"
#include "../lvgl-mvc/events.hpp"

class WeatherHourViewController : public ViewController
{
   protected:
    EventSubscription eventSubscription;

    lv_obj_t *createView(lv_obj_t *parent);
    lv_obj_t *temperatureControll;
    lv_obj_t *iconControll;
    lv_obj_t *timeControll;
    lv_obj_t *windControll;
    lv_obj_t *rainControll;
    lv_obj_t *windDirControll;
    int forecastIndex;

   public:
    WeatherHourViewController(ViewController *parentViewController, int forecastIndex);

    void update();

    void onPushed();
    void onPopped();
};
