#include "wifi-model.hpp"

#include <cstring>

WifiModel::WifiModel() : events(*this)
{
    strncpy(ipAddress, "-", sizeof(ipAddress));
    strncpy(ssid, "-", sizeof(ssid));
    status = WifiStatus::INACTIVE;
}

WifiModel::~WifiModel() {}

void WifiModel::setIpAddress(const char *newIpAddress)
{
    strncpy(ipAddress, newIpAddress, sizeof(ipAddress));
    ipAddress[sizeof(ipAddress) - 1] = 0;
    events.send(WifiModelEvents::ADDRESS_CHANGED, NULL);
}

const char *WifiModel::getIpAddress()
{
    return this->ipAddress;
}

void WifiModel::setSsid(const char *newSsid)
{
    strncpy(ssid, newSsid, sizeof(ssid));
    ssid[sizeof(ssid) - 1] = 0;
    events.send(WifiModelEvents::SSID_CHANGED, NULL);
}

const char *WifiModel::getSsid()
{
    return this->ssid;
}

void WifiModel::setStatus(WifiStatus newStatus)
{
    status = newStatus;
    events.send(WifiModelEvents::STATUS_CHANGED, NULL);
}

WifiStatus WifiModel::getStatus()
{
    return status;
}
