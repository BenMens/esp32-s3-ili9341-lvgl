#pragma once

#include "../lvgl-mvc/view-controller.hpp"
#include "../model/energy-model.hpp"
#include "back-button-controller.hpp"
#include "energy-meter-controller.hpp"

extern EnergyModel energyModel;

class EnergyViewController : public ViewController
{
   private:
    BackButtonViewController backButtonViewController;
    EnergyMeterViewController meter1;
    EnergyMeterViewController meter2;
    EventSubscription energyModelRegistration;

   protected:
    lv_obj_t *createView(lv_obj_t *parent);

   public:
    EnergyViewController(ViewController *parentViewController);

    void onPushed();
    void onPopped();
    void update();
};
