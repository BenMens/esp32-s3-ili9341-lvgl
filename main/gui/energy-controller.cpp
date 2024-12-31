#include "energy-controller.hpp"

#include <cstring>

EnergyViewController::EnergyViewController(ViewController *parentViewController)
    : ViewController(parentViewController),
      backButtonViewController(this),
      meter1(this, 4),
      meter2(this, 4)
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

void EnergyViewController::onPushed()
{
    energyModelRegistration = energyModel.events.addHandler(
        EnergyModelEvents::POWER_DELIVERED_CHANGED |
            EnergyModelEvents::POWER_RETURNED_CHANGED,
        nullptr,
        [&](EnergyModel &source, EnergyModelEvents event,
            EnergyModelEventData eventData, void *userData) { update(); });
}

void EnergyViewController::onPopped()
{
    energyModel.events.removeHandler(energyModelRegistration);
}

void EnergyViewController::update()
{
    if (!viewValid()) return;

    if (lvgl_mvc_lock(0)) {
        meter1.setValue(energyModel.getPowerDelivered());
        meter2.setValue(energyModel.getPowerReturned());

        lvgl_mvc_unlock();
    }
}
