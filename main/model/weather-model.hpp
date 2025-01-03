#pragma once

#include <string.h>

#include "../lvgl-mvc/events.hpp"

enum class WeatherModelEvents : uint16_t {
    FORECAST_CHANGED = 1 << 1,
};
DEFINE_EVENTS_ENUM(WeatherModelEvents)

typedef void *WeatherModelEventData;

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

        strcpy(forecastHour[0].time, "10:00");
        strcpy(forecastHour[1].time, "11:00");
        strcpy(forecastHour[2].time, "12:00");
        strcpy(forecastHour[3].time, "13:00");
        strcpy(forecastHour[4].time, "14:00");
        strcpy(forecastHour[5].time, "15:00");
        strcpy(forecastHour[6].time, "16:00");
        strcpy(forecastHour[7].time, "17:00");
        strcpy(forecastHour[8].time, "18:00");
        strcpy(forecastHour[9].time, "19:00");
        strcpy(forecastHour[10].time, "20:00");
        strcpy(forecastHour[11].time, "21:00");

        strcpy(forecastHour[0].icon, "zonnig");
        strcpy(forecastHour[1].icon, "bliksem");
        strcpy(forecastHour[2].icon, "regen");
        strcpy(forecastHour[3].icon, "buien");
        strcpy(forecastHour[4].icon, "hagel");
        strcpy(forecastHour[5].icon, "mist");
        strcpy(forecastHour[6].icon, "sneeuw");
        strcpy(forecastHour[7].icon, "bewolkt");
        strcpy(forecastHour[8].icon, "lichtbewolkt");
        strcpy(forecastHour[9].icon, "halfbewolkt");
        strcpy(forecastHour[10].icon, "halfbewolkt_regen");
        strcpy(forecastHour[11].icon, "zwaarbewolkt");
    }

    ForecastHour *getForecasthour()
    {
        return forecastHour;
    }
};
