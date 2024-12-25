#pragma once

#include "../lvgl-mvc/events.hpp"

enum class WifiModelEvents : uint16_t {
    STATUS_CHANGED = 1 << 0,
    ADDRESS_CHANGED = 1 << 1,
    SSID_CHANGED = 1 << 2,
};
DEFINE_EVENTS_ENUM(WifiModelEvents)

typedef void *WifiModelEventData;

enum class WifiStatus {
    INACTIVE,
    SCANNING,
    CONNECTION,
    CONNECTED,
    PROVISIONING,
};

class WifiModel
{
   private:
    char ipAddress[16];
    char ssid[33];
    WifiStatus status;

   public:
    Events<WifiModel, WifiModelEvents, WifiModelEventData> events;

    WifiModel();
    ~WifiModel();

    void setIpAddress(const char *newIpAddress);
    const char *getIpAddress();

    void setSsid(const char *newIpAddress);
    const char *getSsid();

    void setStatus(WifiStatus newStatus);
    WifiStatus getStatus();
};
