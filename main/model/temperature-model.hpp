#pragma once

#include <string.h>

#include "../lvgl-mvc/events.hpp"

enum class TemperatureModelEvents : uint16_t {
    MEASUREMENT_CHANGED = 1 << 1,
};
DEFINE_EVENTS_ENUM(TemperatureModelEvents)

typedef struct TemperatureModelEventData {
} TemperatureModelEventData;

class TemperatureModel
{
   private:
    float temperature, humidity;

   public:
    Events<TemperatureModel, TemperatureModelEvents, TemperatureModelEventData>
        events;

    TemperatureModel() : events(*this) {}

    const float& getTemperature()
    {
        return this->temperature;
    }

    const float& getHumidity()
    {
        return this->humidity;
    }

    void setMeasurents(float temperature, float humidity)
    {
        this->temperature = temperature;
        this->humidity = humidity;

        TemperatureModelEventData eventData = {};
        events.send(TemperatureModelEvents::MEASUREMENT_CHANGED, &eventData);
    }
};
