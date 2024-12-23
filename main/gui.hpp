#ifndef __GUI_HPP
#define __GUI_HPP

#include "driver/spi_master.h"
#include "esp_event.h"
#include "esp_lcd_types.h"
#include "lvgl.h"

extern lv_obj_t *hour_hand;
extern lv_obj_t *minute_hand;
extern lv_obj_t *second_hand;
extern lv_obj_t *clock;
extern lv_obj_t *provisioning_qr;

void createGui();

#endif