#include "home-controller.hpp"

#include "../lvgl-mvc/navigation.hpp"
#include "../model/wifi-model.hpp"
#include "clock-controller.hpp"
#include "energy-controller.hpp"
#include "wifi-controller.hpp"

extern WifiModel wifiModel;

HomeViewController::HomeViewController(ViewController *parentViewController)
    : ViewController(parentViewController)
{
}

lv_obj_t *HomeViewController::createView(lv_obj_t *parent)
{
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_flex_flow(&style, LV_FLEX_FLOW_ROW_WRAP);
    lv_style_set_flex_main_place(&style, LV_FLEX_ALIGN_SPACE_EVENLY);
    lv_style_set_layout(&style, LV_LAYOUT_FLEX);

    lv_obj_t *view = lv_obj_create(parent);
    lv_obj_set_size(view, lv_pct(100), lv_pct(100));
    lv_obj_add_style(view, &style, 0);

    lv_obj_t *button;
    lv_obj_t *label;

    // Wifi button

    button = lv_button_create(view);
    lv_obj_set_height(button, lv_pct(25));
    lv_obj_set_flex_grow(button, 1);
    label = lv_label_create(button);
    lv_label_set_text(label, "WiFi");
    lv_obj_set_align(label, LV_ALIGN_CENTER);

    lv_obj_add_event_cb(
        button,
        [](lv_event_t *e) {
            HomeViewController *controller =
                (HomeViewController *)lv_event_get_user_data(e);

            WifiViewController *wifiViewController =
                new WifiViewController(NULL, wifiModel);

            controller->getNavigationontroller()->pushViewController(
                *wifiViewController);
        },
        LV_EVENT_CLICKED, this);

    // Clock button

    button = lv_button_create(view);
    lv_obj_set_height(button, lv_pct(25));
    lv_obj_set_flex_grow(button, 1);

    label = lv_label_create(button);
    lv_label_set_text(label, "Clock");
    lv_obj_set_align(label, LV_ALIGN_CENTER);

    lv_obj_add_event_cb(
        button,
        [](lv_event_t *e) {
            HomeViewController *controller =
                (HomeViewController *)lv_event_get_user_data(e);

            ClockViewController *clockViewController =
                new ClockViewController(NULL);

            controller->getNavigationontroller()->pushViewController(
                *clockViewController);
        },
        LV_EVENT_CLICKED, this);

    // Energy button

    button = lv_button_create(view);
    lv_obj_set_height(button, lv_pct(25));
    lv_obj_set_flex_grow(button, 1);

    label = lv_label_create(button);
    lv_label_set_text(label, "Energy");
    lv_obj_set_align(label, LV_ALIGN_CENTER);

    lv_obj_add_event_cb(
        button,
        [](lv_event_t *e) {
            EnergyViewController *controller =
                (EnergyViewController *)lv_event_get_user_data(e);

            EnergyViewController *energyViewController =
                new EnergyViewController(NULL);

            controller->getNavigationontroller()->pushViewController(
                *energyViewController);
        },
        LV_EVENT_CLICKED, this);

    return view;
}

void HomeViewController::onChildPopped(ViewController *poppedViewController)
{
    delete poppedViewController;
}
