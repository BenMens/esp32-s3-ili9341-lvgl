#pragma once

#include "../lvgl-mvc/view-controller.hpp"
#include "../model/wifi-model.hpp"
#include "back-button-controller.hpp"

class WifiViewController : public ViewController
{
   private:
    WifiModel &wifiModel;
    EventSubscription wifiModelRegistration;
    lv_obj_t *ipAddresLabel;
    lv_obj_t *ssidLabel;
    lv_obj_t *statusLabel;
    lv_obj_t *qrCode;
    BackButtonViewController backButtonViewController;

   protected:
    lv_obj_t *createView(lv_obj_t *parent);

   public:
    WifiViewController(ViewController *parentViewController, WifiModel &model);
    ~WifiViewController();

    void update();
};
