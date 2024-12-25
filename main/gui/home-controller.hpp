#pragma once

#include "../lvgl-mvc/view-controller.hpp"

class HomeViewController : public ViewController
{
   protected:
    lv_obj_t *createView(lv_obj_t *parent);

   public:
    HomeViewController(ViewController *parentViewController);
    void onChildPopped(ViewController *poppedViewController);
};
