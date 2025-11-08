#pragma once

#include "../model/temperature-model.hpp"
#include "../lvgl-mvc/view-controller.hpp"
#include "back-button-controller.hpp"

class ClockViewController : public ViewController
{
   private:
    TemperatureModel &temperatureModel;
    EventSubscription temperatureModelRegistration;
    lv_obj_t *hour_hand;
    lv_obj_t *minute_hand;
    lv_obj_t *second_hand;
    lv_obj_t *clock;
    lv_obj_t *temperatureLabel;
    lv_obj_t *humidityLabel;

    lv_point_precise_t minute_hand_points[2];
    lv_point_precise_t second_hand_points[2];
    lv_timer_t *timer;

    BackButtonViewController backButtonViewController;


   protected:
    lv_obj_t *createView(lv_obj_t *parent);

   public:
    ClockViewController(ViewController *parentViewController,
                        TemperatureModel &emperatureModel);

    void onDidAppear();
    void onWillDisappear();
    void update();
};
