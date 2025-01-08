#include <esp_event.h>
#include <esp_log.h>
#include <esp_lvgl_port.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include <lvgl.h>
#include <nvs_flash.h>
#include <stdio.h>
#include <string.h>
#include <wifi_provisioning/manager.h>
#include <wifi_provisioning/scheme_softap.h>

#include <wifi.hpp>

#include "model/wifi-model.hpp"

WifiModel wifiModel;

static const char *TAG = "app";

#define EXAMPLE_PROV_SEC2_USERNAME "wifiprov"
#define EXAMPLE_PROV_SEC2_PWD "abcd1234"

/* Signal Wi-Fi events on this event-group */
const int WIFI_CONNECTED_EVENT = BIT0;
const int WIFI_FAIL_EVENT = BIT1;
static EventGroupHandle_t wifi_event_group;

#define PROV_QR_VERSION "v1"
#define PROV_TRANSPORT_SOFTAP "softap"

/* Event handler for catching system events */
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_PROV_EVENT) {
        switch (event_id) {
            case WIFI_PROV_START:
                ESP_LOGI(TAG, "Provisioning started");
                wifiModel.setStatus(WifiStatus::PROVISIONING);
                break;
            case WIFI_PROV_CRED_RECV: {
                wifi_sta_config_t *wifi_sta_cfg =
                    (wifi_sta_config_t *)event_data;
                ESP_LOGI(TAG,
                         "Received Wi-Fi credentials"
                         "\n\tSSID     : %s\n\tPassword : %s",
                         (const char *)wifi_sta_cfg->ssid,
                         (const char *)wifi_sta_cfg->password);
                wifiModel.setStatus(WifiStatus::PROVISIONING_CRED_RECV);
                break;
            }
            case WIFI_PROV_CRED_FAIL: {
                wifi_prov_sta_fail_reason_t *reason =
                    (wifi_prov_sta_fail_reason_t *)event_data;
                ESP_LOGE(TAG,
                         "Provisioning failed!\n\tReason : %s"
                         "\n\tPlease reset to factory and retry provisioning",
                         (*reason == WIFI_PROV_STA_AUTH_ERROR)
                             ? "Wi-Fi station authentication failed"
                             : "Wi-Fi access-point not found");

                ESP_LOGI(TAG,
                         "Failed to connect with provisioned AP, resetting "
                         "provisioned credentials");

                wifiModel.setStatus(WifiStatus::PROVISIONING_CRED_FAIL);
                wifi_prov_mgr_reset_sm_state_on_failure();
                break;
            }
            case WIFI_PROV_CRED_SUCCESS:
                ESP_LOGI(TAG, "Provisioning successful");
                wifiModel.setStatus(WifiStatus::PROVISIONING_CRED_SUCCESS);

                break;
            case WIFI_PROV_END:
                /* De-initialize manager once provisioning is finished */
                wifi_prov_mgr_deinit();
                wifiModel.setStatus(WifiStatus::PROVISIONING_CRED_END);
                break;
            default:
                break;
        }
    } else if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                wifiModel.setStatus(WifiStatus::CONNECTING);
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_STOP:
                wifiModel.setStatus(WifiStatus::INACTIVE);
                break;
            case WIFI_EVENT_STA_CONNECTED:
                wifiModel.setStatus(WifiStatus::CONNECTED);
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(TAG, "Disconnected. Connecting to the AP again...");
                xEventGroupSetBits(wifi_event_group, WIFI_FAIL_EVENT);
                esp_wifi_connect();
                wifiModel.setStatus(WifiStatus::CONNECTING);
                break;
            case WIFI_EVENT_AP_STACONNECTED:
                ESP_LOGI(TAG, "SoftAP transport: Connected!");
                break;
            case WIFI_EVENT_AP_STADISCONNECTED:
                ESP_LOGI(TAG, "SoftAP transport: Disconnected!");
                break;
            default:
                break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Connected with IP Address:" IPSTR,
                 IP2STR(&event->ip_info.ip));

        char ifAddressString[16];
        sprintf(ifAddressString, IPSTR, IP2STR(&event->ip_info.ip));
        wifiModel.setIpAddress(ifAddressString);

        wifi_ap_record_t ap_info;
        esp_wifi_sta_get_ap_info(&ap_info);

        wifiModel.setSsid((const char *)ap_info.ssid);

        /* Signal main application to continue execution */
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_EVENT);

    } else if (event_base == PROTOCOMM_SECURITY_SESSION_EVENT) {
        switch (event_id) {
            case PROTOCOMM_SECURITY_SESSION_SETUP_OK:
                ESP_LOGI(TAG, "Secured session established!");
                break;
            case PROTOCOMM_SECURITY_SESSION_INVALID_SECURITY_PARAMS:
                ESP_LOGE(TAG,
                         "Received invalid security parameters for "
                         "establishing secure session!");
                break;
            case PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH:
                ESP_LOGE(TAG,
                         "Received incorrect username and/or PoP for "
                         "establishing secure session!");
                break;
            default:
                break;
        }
    }
}

static void get_device_service_name(char *service_name, size_t max)
{
    uint8_t eth_mac[6];
    const char *ssid_prefix = "PROV_";
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(service_name, max, "%s%02X%02X%02X", ssid_prefix, eth_mac[3],
             eth_mac[4], eth_mac[5]);
}

void provision_wifi_init()
{
    /* Make sure that wifi is stopped */
    esp_wifi_stop();

    /* Configuration for the provisioning manager */
    wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_softap,
        .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE,
        .app_event_handler = WIFI_PROV_EVENT_HANDLER_NONE,
    };

    /* Initialize provisioning manager with the
     * configuration parameters set above */
    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));
}

void provision_wifi()
{
    ESP_LOGI(TAG, "Starting provisioning");

    char service_name[12];
    get_device_service_name(service_name, sizeof(service_name));

    wifi_prov_security_t security = WIFI_PROV_SECURITY_1;
    const char *username = EXAMPLE_PROV_SEC2_USERNAME;
    const char *pop = EXAMPLE_PROV_SEC2_PWD;
    wifi_prov_security1_params_t *sec_params = pop;
    const char *service_key = NULL;
    ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(
        security, (const void *)sec_params, service_name, service_key));

    char payload[150] = {0};
    snprintf(payload, sizeof(payload),
             "{\"ver\":\"%s\",\"name\":\"%s\""
             ",\"username\":\"%s\",\"pop\":\"%s\",\"transport\":\"%s\"}",
             PROV_QR_VERSION, service_name, username, pop,
             PROV_TRANSPORT_SOFTAP);

    // lvgl_port_lock(-1);
    // lv_qrcode_update(provisioning_qr, payload, strlen(payload));
    // lvgl_port_unlock();
}

void start_wifi(bool waitTillConnected)
{
    /* Initialize the event loop */
    wifi_event_group = xEventGroupCreate();

    /* Register our event handler for Wi-Fi, IP and Provisioning related events
     */
    ESP_ERROR_CHECK(esp_event_handler_register(
        WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(PROTOCOMM_SECURITY_SESSION_EVENT,
                                               ESP_EVENT_ANY_ID, &event_handler,
                                               NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                               &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                               &event_handler, NULL));

    /* Initialize Wi-Fi including netif with default config */
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    provision_wifi_init();

    bool provisioned = false;
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));

    /* If device is not yet provisioned start provisioning service */
    if (!provisioned) {
        provision_wifi();
    } else {
        ESP_LOGI(TAG, "Already provisioned, starting Wi-Fi STA");

        /* We don't need the manager as device is already provisioned,
         * so let's release it's resources */
        wifi_prov_mgr_deinit();

        /* Start Wi-Fi station */
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());
    }

    // provision_wifi_init();
    // provision_wifi();

    if (waitTillConnected) {
        EventBits_t bits = xEventGroupWaitBits(
            wifi_event_group, WIFI_CONNECTED_EVENT | WIFI_FAIL_EVENT, pdFALSE,
            pdFALSE, portMAX_DELAY);

        /* xEventGroupWaitBits() returns the bits before the call returned,
         * hence we can test which event actually happened. */
        if (bits & WIFI_CONNECTED_EVENT) {
            ESP_LOGI(TAG, "connected to Wifi");
        } else if (bits & WIFI_FAIL_EVENT) {
            ESP_LOGI(TAG, "Failed to connect to WiFi");
        } else {
            ESP_LOGE(TAG, "UNEXPECTED EVENT");
        }
    }
}