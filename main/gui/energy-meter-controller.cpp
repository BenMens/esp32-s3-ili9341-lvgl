#include "energy-meter-controller.hpp"

#include <cstring>

EnergyMeterViewController::EnergyMeterViewController(
    ViewController *parentViewController)
    : ViewController(parentViewController)
{
}

lv_obj_t *EnergyMeterViewController::createView(lv_obj_t *parent)
{
    int meterPanelSize = 160;
    int meterSize = 120;

    lv_obj_t *meter = lv_obj_create(parent);
    lv_obj_set_size(meter, meterPanelSize, meterPanelSize);
    lv_obj_set_style_bg_opa(meter, 0, 0);
    lv_obj_set_style_border_width(meter, 0, 0);
    lv_obj_set_style_clip_corner(meter, false, 0);
    lv_obj_add_flag(meter, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_add_event_cb(
        meter, [](lv_event_t *e) { lv_event_set_ext_draw_size(e, 50); },
        LV_EVENT_REFR_EXT_DRAW_SIZE, NULL);

    lv_obj_t *scale = lv_scale_create(meter);
    lv_obj_set_size(scale, meterSize, meterSize);
    lv_obj_set_align(scale, LV_ALIGN_CENTER);
    lv_scale_set_range(scale, 0, 5000);
    lv_scale_set_angle_range(scale, 270);
    lv_scale_set_mode(scale, LV_SCALE_MODE_ROUND_OUTER);
    lv_obj_set_style_arc_width(scale, 0, LV_PART_MAIN);

    lv_obj_t *arc;
    arc = lv_arc_create(meter);
    lv_obj_set_size(arc, meterSize, meterSize);
    lv_arc_set_rotation(arc, 135);
    lv_arc_set_bg_angles(arc, 0, 200);
    lv_obj_set_style_arc_width(arc, 15, LV_PART_MAIN);
    lv_obj_center(arc);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_remove_style(arc, NULL, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc, lv_color_make(0x00, 0xff, 0x00),
                               LV_PART_MAIN);

    arc = lv_arc_create(meter);
    lv_obj_set_size(arc, meterSize, meterSize);
    lv_arc_set_rotation(arc, 135);
    lv_arc_set_bg_angles(arc, 200, 270);
    lv_obj_set_style_arc_width(arc, 16, LV_PART_MAIN);
    lv_obj_center(arc);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_remove_style(arc, NULL, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc, lv_color_make(0xff, 0x00, 0x00),
                               LV_PART_MAIN);

    arc = lv_arc_create(meter);
    lv_obj_set_size(arc, meterSize - 6, meterSize - 6);
    lv_arc_set_rotation(arc, 135);
    lv_arc_set_bg_angles(arc, 0, 270);
    lv_arc_set_range(arc, 0, 5000);
    lv_arc_set_value(arc, 2500);
    lv_obj_set_style_arc_width(arc, 10, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, 10, LV_PART_INDICATOR);
    lv_obj_center(arc);
    lv_obj_set_style_arc_color(arc, lv_color_make(0x00, 0x00, 0x00),
                               LV_PART_MAIN);

    static lv_style_t style1;

    lv_style_init(&style1);
    lv_style_set_radius(&style1, 3);

    lv_obj_t *label;
    label = lv_label_create(meter);
    lv_obj_center(label);
    lv_label_set_text(label, "1000 W");
    lv_obj_set_width(label, 70);
    lv_obj_set_style_pad_all(label, 5, 0);
    lv_obj_set_style_border_color(label, lv_color_make(0x80, 0x80, 0x80), 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_border_width(label, 1, 0);
    lv_obj_add_style(label, &style1, 0);

    return meter;
}

void EnergyMeterViewController::update()
{
    if (!viewValid()) return;
}
