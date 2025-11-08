#include "temperature-sensor.hpp"

#include "ahtxx.h"
#include "esp_log.h"

#define TAG "temp-sensor"

static ahtxx_handle_t dev_hdl;

void initTemperatureSensor(i2c_master_bus_handle_t &bus_handle)
{
    ahtxx_config_t dev_cfg = I2C_AHT10_CONFIG_DEFAULT;

    ahtxx_init(bus_handle, &dev_cfg, &dev_hdl);
}

void readTemperatureSensor(TemperatureModel &temperatureModel)
{
    float temperature, humidity;
    esp_err_t result = ahtxx_get_measurement(dev_hdl, &temperature, &humidity);

    if (result != ESP_OK) {
        ESP_LOGE(TAG, "ahtxx device read failed (%s)", esp_err_to_name(result));
    } else {
        temperatureModel.setMeasurents(temperature, humidity);
    }
}