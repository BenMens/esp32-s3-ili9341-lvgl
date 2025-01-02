#pragma once

#include "../lvgl-mvc/events.hpp"

enum class EnergyModelEvents : uint16_t {
    POWER_DELIVERED_CHANGED = 1 << 0,
    POWER_RETURNED_CHANGED = 1 << 1,
    ELECTRICITY_DELIVERED_TODAY_CHANGED = 1 << 1,
    ELECTRICITY_RETURNED_TODAY_CHANGED = 1 << 1,
    GAS_DELIVERED_TODAY_CHANGED = 1 << 1,
};
DEFINE_EVENTS_ENUM(EnergyModelEvents)

typedef void *EnergyModelEventData;

class EnergyModel
{
   private:
    float powerDelivered = 0;
    float powerReturend = 0;
    float electricityDeliveredToday = 0;
    float electricityReturnedToday = 0;
    float gasDeliveredToday = 0;

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

    void setElectricityDeliveredToday(float electricityDeliveredToday)
    {
        this->electricityDeliveredToday = electricityDeliveredToday;
        events.send(EnergyModelEvents::ELECTRICITY_DELIVERED_TODAY_CHANGED,
                    NULL);
    }

    float getElectricityDeliveredToday()
    {
        return this->electricityDeliveredToday;
    }

    void setElectricityReturnedToday(float electricityReturnedToday)
    {
        this->electricityReturnedToday = electricityReturnedToday;
        events.send(EnergyModelEvents::ELECTRICITY_RETURNED_TODAY_CHANGED,
                    NULL);
    }

    float getElectricityReturnedToday()
    {
        return this->electricityReturnedToday;
    }

    void setGasDeliveredToday(float gasDeliveredToday)
    {
        this->gasDeliveredToday = gasDeliveredToday;
        events.send(EnergyModelEvents::GAS_DELIVERED_TODAY_CHANGED, NULL);
    }

    float getGasDeliveredToday()
    {
        return this->gasDeliveredToday;
    }
};
