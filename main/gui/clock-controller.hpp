#pragma once

#include "../lvgl-mvc/view-controller.hpp"
#include "back-button-controller.hpp"

class ClockViewController : public ViewController
{
   private:
    lv_obj_t *hour_hand;
    lv_obj_t *minute_hand;
    lv_obj_t *second_hand;
    lv_obj_t *clock;

    lv_point_precise_t minute_hand_points[2];
    lv_point_precise_t second_hand_points[2];
    lv_timer_t *timer;

    BackButtonViewController backButtonViewController;

   protected:
    lv_obj_t *createView(lv_obj_t *parent);

   public:
    ClockViewController(ViewController *parentViewController);
    ~ClockViewController();

    void update();
};
