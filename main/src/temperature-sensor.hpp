#pragma once

#include "driver/i2c_master.h"
#include "model/temperature-model.hpp"

void readTemperatureSensor(TemperatureModel &temperatureModel);

void initTemperatureSensor(i2c_master_bus_handle_t &bus_handle);