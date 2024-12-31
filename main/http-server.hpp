#pragma once

#include <esp_http_server.h>

esp_err_t startWebserver(httpd_handle_t *handle, const char *basePath);

void stopWebserver(httpd_handle_t server);