#include "home-controller.hpp"

#include "../lvgl-mvc/navigation.hpp"
#include "../model/wifi-model.hpp"
#include "clock-controller.hpp"
#include "energy-controller.hpp"
#include "weather-controller.hpp"
#include "wifi-controller.hpp"

extern WifiModel wifiModel;

HomeViewController::HomeViewController(ViewController *parentViewController)
    : ViewController(parentViewController)
{
}

lv_obj_t *HomeViewController::createView(lv_obj_t *parent)
{
    static const char *btnm_map[] = {"WiFi",   "Clock",   "\n",
                                     "Energy", "Weather", NULL};

    lv_obj_t *btnMatrix = lv_buttonmatrix_create(parent);
    lv_obj_set_size(btnMatrix, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(btnMatrix, 8, LV_PART_MAIN);
    lv_btnmatrix_set_map(btnMatrix, btnm_map);
    lv_obj_set_style_bg_color(btnMatrix, lv_theme_get_color_primary(btnMatrix),
                              LV_PART_ITEMS);

    lv_obj_set_style_bg_opa(btnMatrix, LV_OPA_COVER, LV_PART_ITEMS);
    lv_obj_set_style_radius(btnMatrix, 5, LV_PART_ITEMS);

    lv_obj_add_event_cb(
        btnMatrix,
        [](lv_event_t *e) {
            HomeViewController *controller =
                (HomeViewController *)lv_event_get_user_data(e);
            lv_event_code_t code = lv_event_get_code(e);
            if (code == LV_EVENT_VALUE_CHANGED) {
                lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
                uint32_t id = lv_buttonmatrix_get_selected_button(obj);

                switch (id) {
                    case 0:
                        controller->getNavigationontroller()
                            ->pushViewController(
                                *new WifiViewController(NULL, wifiModel));
                        break;
                    case 1:
                        controller->getNavigationontroller()
                            ->pushViewController(
                                *new ClockViewController(NULL));
                        break;
                    case 2:
                        controller->getNavigationontroller()
                            ->pushViewController(
                                *new EnergyViewController(NULL));
                        break;
                    case 3:
                        controller->getNavigationontroller()
                            ->pushViewController(
                                *new WeatherViewController(NULL));
                        break;
                }
            }
        },
        LV_EVENT_ALL, this);

    return btnMatrix;
}

void HomeViewController::onChildPopped(ViewController *poppedViewController)
{
    delete poppedViewController;
}
