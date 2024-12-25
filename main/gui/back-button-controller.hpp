#pragma once

#include "../lvgl-mvc/view-controller.hpp"

class BackButtonViewController : public ViewController
{
   protected:
    lv_obj_t *createView(lv_obj_t *parent);

   public:
    BackButtonViewController(ViewController *parentViewController);
};
