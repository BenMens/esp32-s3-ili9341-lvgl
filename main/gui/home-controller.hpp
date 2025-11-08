#pragma once

#include "../lvgl-mvc/view-controller.hpp"
#include "../model/energy-model.hpp"
#include "../model/temperature-model.hpp"
#include "../model/weather-model.hpp"
#include "../model/wifi-model.hpp"

class HomeViewController : public ViewController
{
   protected:
    lv_obj_t *createView(lv_obj_t *parent);
    WeatherModel &weatherModel;
    WifiModel &wifiModel;
    EnergyModel &energyModel;
    TemperatureModel &temperatureModel;

   public:
    HomeViewController(ViewController *parentViewController,
                       WeatherModel &weatherModel, WifiModel &wifiModel,
                       EnergyModel &energyModel,
                       TemperatureModel &temperatureModel);
    void onChildPopped(ViewController *poppedViewController);
};
