#pragma once

#include "../lvgl-mvc/events.hpp"

enum class EnergyModelEvents : uint16_t {
    POWER_DELIVERED_CHANGED = 1 << 0,
    POWER_RETURNED_CHANGED = 1 << 1,
};
DEFINE_EVENTS_ENUM(EnergyModelEvents)

typedef void *EnergyModelEventData;

class EnergyModel
{
   private:
    float powerDelivered = 0;
    float powerReturend = 0;

   public:
    Events<EnergyModel, EnergyModelEvents, EnergyModelEventData> events;

    EnergyModel() : events(*this) {}

    void setPowerDelivered(float powerDelivered)
    {
        this->powerDelivered = powerDelivered;
        events.send(EnergyModelEvents::POWER_DELIVERED_CHANGED, NULL);
    }

    float getPowerDelivered()
    {
        return this->powerDelivered;
    }

    void setPowerReturned(float powerReturned)
    {
        this->powerReturend = powerReturned;
        events.send(EnergyModelEvents::POWER_RETURNED_CHANGED, NULL);
    }

    float getPowerReturned()
    {
        return this->powerReturend;
    }
};
