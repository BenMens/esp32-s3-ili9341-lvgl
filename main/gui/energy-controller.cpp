#include "energy-controller.hpp"

#include <cstring>

EnergyViewController::EnergyViewController(ViewController *parentViewController)
    : ViewController(parentViewController),
      backButtonViewController(this),
      meter1(this),
      meter2(this)
{
}

lv_obj_t *EnergyViewController::createView(lv_obj_t *parent)
{
    lv_obj_t *view = lv_obj_create(parent);
    lv_obj_set_size(view, lv_pct(100), lv_pct(100));

    lv_obj_t *m1 = meter1.getViewAttachedToParent(view);
    lv_obj_set_align(m1, LV_ALIGN_BOTTOM_LEFT);

    lv_obj_t *m2 = meter2.getViewAttachedToParent(view);
    lv_obj_set_align(m2, LV_ALIGN_BOTTOM_RIGHT);

    backButtonViewController.getViewAttachedToParent(view);

    return view;
}

void EnergyViewController::update()
{
    if (!viewValid()) return;
}
