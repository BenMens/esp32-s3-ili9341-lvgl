#pragma once

#include <string.h>

#include "../lvgl-mvc/events.hpp"

enum class WeatherModelEvents : uint16_t {
    FORECAST_CHANGED = 1 << 1,
};
DEFINE_EVENTS_ENUM(WeatherModelEvents)

typedef struct WeatherModelEventData {
    int index;
} WeatherModelEventData;

struct ForecastHour {
    char time[6];
    char icon[20];
    float temperature;
};

class WeatherModel
{
   private:
    ForecastHour forecastHour[12];

   public:
    Events<WeatherModel, WeatherModelEvents, WeatherModelEventData> events;

    WeatherModel() : events(*this)
    {
        for (int i = 0; i < sizeof(forecastHour) / sizeof(ForecastHour); i++) {
            forecastHour[i].temperature = 0;
        }
    }

    const ForecastHour& getForecasthour(int index)
    {
        return forecastHour[index];
    }

    void setForecasthour(int index, ForecastHour& forecast)
    {
        forecastHour[index] = forecast;

        WeatherModelEventData eventData = {.index = index};
        events.send(WeatherModelEvents::FORECAST_CHANGED, &eventData);
    }
};
