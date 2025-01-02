#include "mqtt-client.hpp"

#include <algorithm>

#include "esp_log.h"
#include "model/energy-model.hpp"
#include "mqtt_client.h"

extern EnergyModel energyModel;

#define CONFIG_BROKER_URL "mqtt://192.168.1.123:1883"
#define TAG "mqtt-client"

static void log_error_if_nonzero(const char* message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void* handler_args, esp_event_base_t base,
                               int32_t event_id, void* event_data)
{
    ESP_LOGD(TAG,
             "Event dispatched from event loop base=%s, event_id=%" PRIi32 "",
             base, event_id);
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

            msg_id = esp_mqtt_client_subscribe(client,
                                               "p1/actuals/electricity/+", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "p1/actuals/gas/+", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA: {
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");

            char strBuf[20];
            if (event->data_len >= sizeof(strBuf)) {
                ESP_LOGE(TAG, "Topic datasze exceeds buffer size (%.*s)",
                         event->data_len, event->data);
                break;
            }
            ESP_LOGI(TAG, "(topic, data)=(%.*s, %.*s)", event->topic_len,
                     event->topic, event->data_len, event->data);

            memcpy(strBuf, event->data, event->data_len);
            strBuf[event->data_len] = 0;

            if (strncmp(event->topic, "p1/actuals/electricity/delivered",
                        event->topic_len) == 0) {
                float value = atof(strBuf);

                energyModel.setPowerDelivered(value);
            } else if (strncmp(event->topic, "p1/actuals/electricity/returned",
                               event->topic_len) == 0) {
                float value = atof(strBuf);

                energyModel.setPowerReturned(value);
            } else if (strncmp(event->topic,
                               "p1/actuals/electricity/deliveredToday",
                               event->topic_len) == 0) {
                float value = atof(strBuf);

                energyModel.setElectricityDeliveredToday(value);
            } else if (strncmp(event->topic,
                               "p1/actuals/electricity/returnedToday",
                               event->topic_len) == 0) {
                float value = atof(strBuf);

                energyModel.setElectricityReturnedToday(value);
            } else if (strncmp(event->topic, "p1/actuals/gas/usedToday",
                               event->topic_len) == 0) {
                float value = atof(strBuf);

                energyModel.setGasDeliveredToday(value);
            }
        } break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");

            if (event->error_handle->error_type ==
                MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                log_error_if_nonzero("reported from esp-tls",
                                     event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack",
                                     event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero(
                    "captured as transport's socket errno",
                    event->error_handle->esp_transport_sock_errno);
                ESP_LOGI(
                    TAG, "Last errno string (%s)",
                    strerror(event->error_handle->esp_transport_sock_errno));
            }
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}

void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg;
    memset(&mqtt_cfg, 0, sizeof(mqtt_cfg));
    mqtt_cfg.broker.address.uri = CONFIG_BROKER_URL;

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(client,
                                   (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID,
                                   mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}