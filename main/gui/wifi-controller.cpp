#include "wifi-controller.hpp"

#include <stdio.h>

#include <cstring>

WifiViewController::WifiViewController(ViewController *parentViewController,
                                       WifiModel &wifiModel)
    : ViewController(parentViewController),
      wifiModel(wifiModel),
      backButtonViewController(this)
{
}

lv_obj_t *WifiViewController::createView(lv_obj_t *parent)
{
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_flex_flow(&style, LV_FLEX_FLOW_COLUMN_WRAP);
    lv_style_set_flex_main_place(&style, LV_FLEX_ALIGN_START);
    lv_style_set_layout(&style, LV_LAYOUT_FLEX);
    lv_style_set_bg_opa(&style, 0);
    lv_style_set_border_width(&style, 0);

    lv_obj_t *view = lv_obj_create(parent);
    lv_obj_set_size(view, lv_pct(100), lv_pct(100));

    lv_obj_t *contentView = lv_obj_create(view);
    lv_obj_set_size(contentView, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_add_style(contentView, &style, 0);
    lv_obj_set_style_align(contentView, LV_ALIGN_BOTTOM_RIGHT, 0);

    lv_obj_t *label;

    label = lv_label_create(contentView);
    lv_label_set_text(label, "ip address:");
    ipAddresLabel = lv_label_create(contentView);
    lv_obj_set_style_margin_left(ipAddresLabel, 20, 0);

    label = lv_label_create(contentView);
    lv_label_set_text(label, "ssid:");
    ssidLabel = lv_label_create(contentView);
    lv_obj_set_style_margin_left(ssidLabel, 20, 0);

    label = lv_label_create(contentView);
    lv_label_set_text(label, "status");
    statusLabel = lv_label_create(contentView);
    lv_obj_set_style_margin_left(statusLabel, 20, 0);

    qrCode = lv_qrcode_create(view);
    lv_obj_set_style_align(qrCode, LV_ALIGN_LEFT_MID, 0);
    lv_obj_set_style_pad_all(qrCode, 30, 0);
    lv_qrcode_set_size(qrCode, 150);

    backButtonViewController.getViewAttachedToParent(view);

    return view;
}

void WifiViewController::onPushed()
{
    wifiModelRegistration = wifiModel.events.addHandler(
        WifiModelEvents::ADDRESS_CHANGED | WifiModelEvents::SSID_CHANGED |
            WifiModelEvents::STATUS_CHANGED,
        nullptr,
        [&](WifiModel &source, WifiModelEvents event,
            WifiModelEventData eventData, void *userData) { this->update(); });
}

void WifiViewController::onPopped()
{
    wifiModel.events.removeHandler(wifiModelRegistration);
}

void WifiViewController::update()
{
    if (!viewValid()) return;

    if (lvgl_mvc_lock(0)) {
        lv_label_set_text(ipAddresLabel, wifiModel.getIpAddress());
        lv_label_set_text(ssidLabel, wifiModel.getSsid());
        lv_label_set_text(statusLabel, "-");

        if (strlen(wifiModel.getIpAddress()) > 1) {
            char url[32];

            snprintf(url, sizeof(url), "http://%s", wifiModel.getIpAddress());
            lv_qrcode_update(qrCode, url, strlen(url));
            lv_obj_remove_flag(qrCode, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(qrCode, LV_OBJ_FLAG_HIDDEN);
        }

        lvgl_mvc_unlock();
    }
}
