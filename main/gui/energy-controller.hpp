#pragma once

#include "../lvgl-mvc/view-controller.hpp"
#include "../model/wifi-model.hpp"
#include "back-button-controller.hpp"

class EnergyViewController : public ViewController
{
   private:
    BackButtonViewController backButtonViewController;

   protected:
    lv_obj_t *createView(lv_obj_t *parent);

   public:
    EnergyViewController(ViewController *parentViewController);

    void update();
};
