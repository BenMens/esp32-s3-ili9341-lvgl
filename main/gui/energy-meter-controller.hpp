#pragma once

#include "../lvgl-mvc/view-controller.hpp"

class EnergyMeterViewController : public ViewController
{
   protected:
    lv_obj_t *createView(lv_obj_t *parent);
    float scaleMax;
    lv_obj_t *valueLabel;
    lv_obj_t *valueScale;

   public:
    EnergyMeterViewController(ViewController *parentViewController,
                              float scaleMax);

    void update();
    void setValue(float value);
};
