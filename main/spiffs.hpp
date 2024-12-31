#pragma once

#include "esp_spiffs.h"
#include "esp_err.h"

esp_err_t startSpiffs(const char *basePath);