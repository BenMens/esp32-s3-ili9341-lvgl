#include "back-button-controller.hpp"

#include "../lvgl-mvc/navigation.hpp"

BackButtonViewController::BackButtonViewController(
    ViewController *parentViewController)
    : ViewController(parentViewController)
{
}

lv_obj_t *BackButtonViewController::createView(lv_obj_t *parent)
{
    lv_obj_t *button = lv_button_create(parent);
    lv_obj_t *buttonLabel = lv_label_create(button);
    lv_label_set_text(buttonLabel, LV_SYMBOL_LEFT);
    lv_obj_set_style_align(buttonLabel, LV_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(button, 0, 0);
    lv_obj_set_style_border_color(button, lv_color_make(0x40, 0x40, 0x40), 0);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_size(button, 60, 60);
    lv_obj_set_style_align(button, LV_ALIGN_BOTTOM_LEFT, 0);

    lv_obj_add_event_cb(
        button,
        [](lv_event_t *e) {
            BackButtonViewController *controller =
                (BackButtonViewController *)lv_event_get_user_data(e);

            controller->getNavigationontroller()->popViewController();
        },
        LV_EVENT_CLICKED, this);

    return button;
}
