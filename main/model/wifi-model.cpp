#include "wifi-model.hpp"

#include <cstring>

WifiModel::WifiModel() : events(*this), getProvisioningQrCodeString(NULL)
{
    strlcpy(ipAddress, "-", sizeof(ipAddress));
    strlcpy(ssid, "-", sizeof(ssid));
    status = WifiStatus::INACTIVE;
}

WifiModel::~WifiModel() {}

void WifiModel::setIpAddress(const char *newIpAddress)
{
    strlcpy(ipAddress, newIpAddress, sizeof(ipAddress));
    events.send(WifiModelEvents::ADDRESS_CHANGED, NULL);
}

const char *WifiModel::getIpAddress()
{
    return this->ipAddress;
}

void WifiModel::setSsid(const char *newSsid)
{
    strlcpy(ssid, newSsid, sizeof(ssid));
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

const char *wifiStatusAsString(WifiStatus status)
{
    switch (status) {
        case WifiStatus::INACTIVE:
            return "INACTIVE";
            break;
        case WifiStatus::SCANNING:
            return "SCANNING";
            break;
        case WifiStatus::CONNECTING:
            return "CONNECTING";
            break;
        case WifiStatus::CONNECTED:
            return "connected";
            break;
        case WifiStatus::PROVISIONING:
            return "PROVISIONING";
            break;
        case WifiStatus::PROVISIONING_CRED_RECV:
            return "PROV_CRED_RECV";
            break;
        case WifiStatus::PROVISIONING_CRED_FAIL:
            return "PROV_CRED_FAIL";
            break;
        default:
            return "undefined";
            break;
    }
}