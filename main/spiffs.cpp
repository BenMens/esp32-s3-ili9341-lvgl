#include "spiffs.hpp"

#include "esp_log.h"

#define TAG "spiffs"

esp_err_t startSpiffs(const char *basePath)
{
    esp_vfs_spiffs_conf_t conf = {.base_path = basePath,
                                  .partition_label = NULL,
                                  .max_files = 5,
                                  .format_if_mount_failed = true};

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)",
                     esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Performing SPIFFS_check().");
    ret = esp_spiffs_check(conf.partition_label);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
        return ESP_FAIL;
    } else {
        ESP_LOGI(TAG, "SPIFFS_check() successful");
    }

    return ESP_OK;
}