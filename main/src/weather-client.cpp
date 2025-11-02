#include "weather-client.hpp"

#include <sys/param.h>

#include "cJSON.h"
#include "esp_heap_caps.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_tls.h"
#include "model/weather-model.hpp"

extern const char isrg_root_x1_pem_start[] asm("_binary_isrg_root_x1_pem_start");
extern const char isrg_root_x1_pem_end[]   asm("_binary_isrg_root_x1_pem_end");

WeatherModel weatherModel;

typedef struct ResponseBodyBuffer {
    void *body;
    int maxBodySize;
    int bodyLength;
    bool bufferOverflow;
} ResponseBodyBuffer;

#define MAX_HTTP_OUTPUT_BUFFER (8 * 1024)

#define TAG "weather-client"

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s",
                     evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA: {
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);

            ResponseBodyBuffer *responseBodyBuffer =
                (ResponseBodyBuffer *)evt->user_data;

            if (responseBodyBuffer->body) {
                if (responseBodyBuffer->bodyLength + evt->data_len >
                    responseBodyBuffer->maxBodySize) {
                    responseBodyBuffer->bufferOverflow = true;
                    ESP_LOGE(TAG, "Buffer overflow when downloading http body");
                } else {
                    if (evt->data_len) {
                        memcpy((char *)responseBodyBuffer->body +
                                   responseBodyBuffer->bodyLength,
                               evt->data, evt->data_len);
                        responseBodyBuffer->bodyLength += evt->data_len;
                    }

                    // null terminate the body if there is room left for it
                    if (responseBodyBuffer->bodyLength <
                        responseBodyBuffer->maxBodySize) {
                        ((char *)responseBodyBuffer
                             ->body)[responseBodyBuffer->bodyLength] = 0;
                    }
                }
            }
        } break;
        case HTTP_EVENT_ON_FINISH: {
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        } break;
        case HTTP_EVENT_DISCONNECTED: {
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error(
                (esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
        } break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
            break;
        default:
            ESP_LOGD(TAG, "unhandled HTTP_EVENT %d", evt->event_id);
            break;
    }

    return ESP_OK;
}

static void http_rest_with_url(void)
{
    ResponseBodyBuffer responseBuffer = {};
    responseBuffer.body =
        heap_caps_calloc(1, MAX_HTTP_OUTPUT_BUFFER, MALLOC_CAP_SPIRAM);
    responseBuffer.maxBodySize = MAX_HTTP_OUTPUT_BUFFER;

    esp_http_client_config_t config = {};

    config.url =
        "https://weerlive.nl/api/"
        "weerlive_api_v2.php?key=2db479ee91&locatie=Zoetermeer";
    // config.url =
    //     "https://weerlive.nl/api/"
    //     "weerlive_api_v2.php?key=demo&locatie=Amsterdam";
    config.event_handler = _http_event_handler;
    config.user_data = &responseBuffer;
    config.disable_auto_redirect = true;
    config.method = HTTP_METHOD_GET;
    config.keep_alive_enable = true;
    config.cert_pem = isrg_root_x1_pem_start;

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK && !responseBuffer.bufferOverflow) {
        ESP_LOGD(TAG, "HTTP GET Status = %d, content_length = %d",
                 esp_http_client_get_status_code(client),
                 responseBuffer.bodyLength);

        cJSON *weather_json = cJSON_Parse((char *)responseBuffer.body);
        cJSON *uur_verw = cJSON_GetObjectItem(weather_json, "uur_verw");

        for (int i = 0; i < 12; i++) {
            cJSON *uur = cJSON_GetArrayItem(uur_verw, i);

            if (uur != NULL) {
                ForecastHour forecast;
                strlcpy(forecast.icon,
                        cJSON_GetObjectItem(uur, "image")->valuestring,
                        sizeof(forecast.icon));
                forecast.temperature =
                    (float)cJSON_GetObjectItem(uur, "temp")->valuedouble;

                forecast.windSpeed =
                    (float)cJSON_GetObjectItem(uur, "windbft")->valuedouble;

                strlcpy(forecast.windDir,
                        cJSON_GetObjectItem(uur, "windr")->valuestring,
                        sizeof(forecast.windDir));

                forecast.rain =
                    (float)cJSON_GetObjectItem(uur, "neersl")->valuedouble;

                char *t =
                    strchr(cJSON_GetObjectItem(uur, "uur")->valuestring, ' ');
                if (t != NULL) {
                    strlcpy(forecast.time, ++t, sizeof(forecast.time));
                }

                weatherModel.setForecasthour(i, forecast);
            } else {
                ESP_LOGE(TAG, "Error parsing wheather data: %s",
                         (char *)responseBuffer.body);
            }
        }

        cJSON_Delete(weather_json);
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);

    heap_caps_free(responseBuffer.body);
}

void readWeatherService(void)
{
    http_rest_with_url();
}