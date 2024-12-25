#pragma once

#include <inttypes.h>

extern bool lvgl_mvc_lock(uint32_t timeout_ms);

extern void lvgl_mvc_unlock(void);
