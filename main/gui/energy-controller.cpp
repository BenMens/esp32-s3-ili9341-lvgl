#include "energy-controller.hpp"

#include <cstring>

EnergyViewController::EnergyViewController(ViewController *parentViewController)
    : ViewController(parentViewController), backButtonViewController(this)
{
}

lv_obj_t *EnergyViewController::createView(lv_obj_t *parent)
{
    lv_obj_t *view = lv_obj_create(parent);
    lv_obj_set_size(view, lv_pct(100), lv_pct(100));

    lv_obj_t *scale = lv_scale_create(view);
    lv_obj_set_size(scale, 150, 150);
    lv_obj_set_align(scale, LV_ALIGN_CENTER);
    lv_scale_set_range(scale, 0, 5000);
    lv_scale_set_angle_range(scale, 200);
    lv_scale_set_mode(scale, LV_SCALE_MODE_ROUND_OUTER);
    lv_obj_set_style_arc_width(scale, 0, LV_PART_MAIN);
    // lv_scale_set_draw_ticks_on_top(scale, true);

    // static lv_style_t main_line_style;
    // lv_style_init(&main_line_style);
    // /* Main line properties */
    // // lv_style_set_line_color(&main_line_style, lv_color_hex(0x0000ff));
    // lv_style_set_line_width(&main_line_style, 20U);  // Tick width
    // lv_style_set_width(&main_line_style, 10U);       // Tick length
    // // lv_obj_add_style(scale, &main_line_style, LV_PART_ITEMS);
    // // lv_obj_add_style(scale, &main_line_style, LV_PART_INDICATOR);

    // static lv_style_t section1_main_line_style;
    // lv_style_init(&section1_main_line_style);
    // lv_style_set_arc_color(&section1_main_line_style,
    //                        lv_color_make(0x00, 0xff, 0x00));

    // static lv_style_t section2_main_line_style;
    // lv_style_init(&section2_main_line_style);
    // lv_style_set_arc_color(&section2_main_line_style,
    //                        lv_color_make(0xff, 0x00, 0x00));

    // lv_scale_section_t *section1 = lv_scale_add_section(scale);
    // lv_scale_section_set_range(section1, 0, 3000);
    // lv_scale_section_set_style(section1, LV_PART_MAIN,
    //                            &section1_main_line_style);

    // lv_scale_section_t *section2 = lv_scale_add_section(scale);
    // lv_scale_section_set_range(section2, 3000, 5000);
    // lv_scale_section_set_style(section2, LV_PART_MAIN,
    //                            &section2_main_line_style);

    // lv_obj_set_style_pad_all(scale, -1, 0);

    //  lv_obj_t * label = lv_label_create(parent);

    lv_obj_t *arc;
    /*Create an Arc*/
    arc = lv_arc_create(view);
    lv_obj_set_size(arc, 150, 150);
    lv_arc_set_rotation(arc, 135);
    lv_arc_set_bg_angles(arc, 0, 150);
    // lv_arc_set_value(arc, 10);
    lv_obj_set_style_arc_width(arc, 15, LV_PART_MAIN);
    lv_obj_center(arc);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_remove_style(arc, NULL, LV_PART_INDICATOR);
    // lv_obj_set_style_arc_rounded(arc, false, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_make(0x00, 0xff, 0x00),
                               LV_PART_MAIN);

    arc = lv_arc_create(view);
    lv_obj_set_size(arc, 150, 150);
    lv_arc_set_rotation(arc, 135);
    lv_arc_set_bg_angles(arc, 150, 200);
    // lv_arc_set_value(arc, 10);
    lv_obj_set_style_arc_width(arc, 16, LV_PART_MAIN);
    lv_obj_center(arc);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_remove_style(arc, NULL, LV_PART_INDICATOR);
    // lv_obj_set_style_arc_rounded(arc, false, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_make(0xff, 0x00, 0x00),
                               LV_PART_MAIN);

    arc = lv_arc_create(view);
    lv_obj_set_size(arc, 144, 144);
    lv_arc_set_rotation(arc, 135);
    lv_arc_set_bg_angles(arc, 0, 200);
    lv_arc_set_value(arc, 10);
    lv_obj_set_style_arc_width(arc, 10, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, 10, LV_PART_INDICATOR);
    lv_obj_center(arc);
    // lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    // lv_obj_remove_style(arc, NULL, LV_PART_INDICATOR);
    // lv_obj_set_style_arc_rounded(arc, false, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_make(0x00, 0x00, 0x00),
                               LV_PART_MAIN);

    static lv_style_t style1;

    lv_style_init(&style1);
    lv_style_set_radius(&style1, 3);

    lv_obj_t *label;
    label = lv_label_create(view);
    lv_obj_center(label);
    lv_label_set_text(label, "1000 W");
    lv_obj_set_width(label, 80);
    lv_obj_set_style_pad_all(label, 5, 0);
    lv_obj_set_style_border_color(label, lv_color_make(0x80, 0x80, 0x80), 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_border_width(label, 1, 0);
    lv_obj_add_style(label, &style1, 0);

    backButtonViewController.getViewAttachedToParent(view);

    return view;
}

void EnergyViewController::update()
{
    if (!viewValid()) return;
}
