#pragma once

#include "../lvgl-mvc/view-controller.hpp"
#include "../model/energy-model.hpp"
#include "back-button-controller.hpp"
#include "energy-meter-controller.hpp"

class EnergyViewController : public ViewController
{
   private:
    EnergyModel &energyModel;
    EventSubscription energyModelRegistration;
    BackButtonViewController backButtonViewController;
    EnergyMeterViewController meter1;
    EnergyMeterViewController meter2;
    lv_obj_t *electricityDeliveredWidget;
    lv_obj_t *electricityReturnedWidget;
    lv_obj_t *gasDeliveredWidget;

   protected:
    lv_obj_t *createView(lv_obj_t *parent);

   public:
    EnergyViewController(ViewController *parentViewController,
                         EnergyModel &energyModel);

    void onDidAppear();
    void onWillDisappear();
    void update();
};
