#include "energy-controller.hpp"

#include <stdio.h>

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

    electricityDeliveredWidget = lv_label_create(view);
    lv_obj_set_pos(electricityDeliveredWidget, 10, 10);

    electricityReturnedWidget = lv_label_create(view);
    lv_obj_align_to(electricityReturnedWidget, electricityDeliveredWidget,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

    gasDeliveredWidget = lv_label_create(view);
    lv_obj_align_to(gasDeliveredWidget, electricityReturnedWidget,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

    backButtonViewController.getViewAttachedToParent(view);

    return view;
}

void EnergyViewController::onPushed()
{
    energyModelRegistration = energyModel.events.addHandler(
        EnergyModelEvents::POWER_DELIVERED_CHANGED |
            EnergyModelEvents::POWER_RETURNED_CHANGED |
            EnergyModelEvents::ELECTRICITY_DELIVERED_TODAY_CHANGED |
            EnergyModelEvents::ELECTRICITY_RETURNED_TODAY_CHANGED |
            EnergyModelEvents::GAS_DELIVERED_TODAY_CHANGED,
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

        char strBuf[30];

        snprintf(strBuf, sizeof(strBuf), "el deliverd: %.2f kWh",
                 energyModel.getElectricityDeliveredToday());
        lv_label_set_text(electricityDeliveredWidget, strBuf);

        snprintf(strBuf, sizeof(strBuf), "el returned: %.2f kWh",
                 energyModel.getElectricityReturnedToday());
        lv_label_set_text(electricityReturnedWidget, strBuf);

        snprintf(strBuf, sizeof(strBuf), "gas deliverd: %.2f m3",
                 energyModel.getGasDeliveredToday());
        lv_label_set_text(gasDeliveredWidget, strBuf);

        lvgl_mvc_unlock();
    }
}
