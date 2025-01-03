#pragma once

#include "../lvgl-mvc/view-controller.hpp"
#include "../model/weather-model.hpp"

class WeatherHourViewController : public ViewController
{
   protected:
    lv_obj_t *createView(lv_obj_t *parent);
    lv_obj_t *temperatureControll;
    lv_obj_t *iconControll;
    lv_obj_t *timeControll;
    ForecastHour *forecast;

   public:
    WeatherHourViewController(ViewController *parentViewController);

    void update();

    void setForecast(ForecastHour *forecast);
};
