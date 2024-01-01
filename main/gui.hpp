#ifndef __GUI_HPP
#define __GUI_HPP

#include "driver/spi_master.h"
#include "esp_event.h"

ESP_EVENT_DECLARE_BASE(GUI_EVENTS);

enum { GUI_BUTTON_EVENT };

typedef union {
    struct {
        uint8_t id;
        uint8_t action;
    } button;

} gui_event_t;

esp_err_t guiInit();

#endif