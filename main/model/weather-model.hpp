#pragma once

#include <string.h>

#include "../lvgl-mvc/events.hpp"

#define WEATHER_NUM_FORECASE_HOURS 12

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
    int windSpeed;
    float rain;
    char windDir[4];
};

class WeatherModel
{
   private:
    ForecastHour forecastHour[WEATHER_NUM_FORECASE_HOURS];

   public:
    Events<WeatherModel, WeatherModelEvents, WeatherModelEventData> events;

    WeatherModel() : events(*this)
    {
        for (int i = 0; i < WEATHER_NUM_FORECASE_HOURS; i++) {
            forecastHour[i].temperature = 0;
            forecastHour[i].rain = 0;
            forecastHour[i].windSpeed = 0;
            forecastHour[i].icon[0] = 0;
            forecastHour[i].time[0] = 0;
            forecastHour[i].windDir[0] = 0;
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
