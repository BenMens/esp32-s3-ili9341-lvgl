#pragma once

#include "../lvgl-mvc/view-controller.hpp"
#include "back-button-controller.hpp"
#include "energy-meter-controller.hpp"

class EnergyViewController : public ViewController
{
   private:
    BackButtonViewController backButtonViewController;
    EnergyMeterViewController meter1;
    EnergyMeterViewController meter2;

   protected:
    lv_obj_t *createView(lv_obj_t *parent);

   public:
    EnergyViewController(ViewController *parentViewController);

    void update();
};
