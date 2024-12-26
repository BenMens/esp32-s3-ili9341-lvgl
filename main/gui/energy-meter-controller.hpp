#pragma once

#include "../lvgl-mvc/view-controller.hpp"

class EnergyMeterViewController : public ViewController
{
   protected:
    lv_obj_t *createView(lv_obj_t *parent);

   public:
    EnergyMeterViewController(ViewController *parentViewController);

    void update();
};
