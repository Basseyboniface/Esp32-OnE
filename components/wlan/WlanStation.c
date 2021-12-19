#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include "Wlan.h"
#include "WlanStation.h"

static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define ESP_MAXIMUM_RETRY 3

static const char *TAG = "WlanSTA";
static int s_retry_num = 0;

esp_event_handler_instance_t instance_any_id;
esp_event_handler_instance_t instance_got_ip;
static esp_netif_t *m_wlan = NULL;

static void WlanStationEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }

    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
            s_retry_num++;
            LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        LOGI(TAG, "connect to AP failed");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t StartWlanStation(const char *ssid, const char *password)
{
    esp_err_t status = ESP_OK;
    s_wifi_event_group = xEventGroupCreate();

    do
    {
        status = InitializeWifi();
        if (status != ESP_OK)
        {
            LOGE(TAG, "initialize wifi failed");
            break;
        }

        m_wlan = esp_netif_create_default_wifi_sta();

        status = esp_event_handler_instance_register(WIFI_EVENT,
                                                     ESP_EVENT_ANY_ID,
                                                     &WlanStationEventHandler,
                                                     NULL,
                                                     &instance_any_id);
        if (status != ESP_OK)
        {
            LOGE(TAG, "register WiFi event failed");
            break;
        }

        status = esp_event_handler_instance_register(IP_EVENT,
                                                     IP_EVENT_STA_GOT_IP,
                                                     &WlanStationEventHandler,
                                                     NULL,
                                                     &instance_got_ip);
        if (status != ESP_OK)
        {
            LOGE(TAG, "register IP event failed");
            break;
        }

        status = esp_wifi_set_mode(WIFI_MODE_STA);
        if (status != ESP_OK)
        {
            LOGE(TAG, "set wifi STA mode failed");
            break;
        }

        wifi_config_t sta_config = {
            .sta = {
                .scan_method = WIFI_FAST_SCAN,
                .bssid_set = false,
                .bssid[0] = '\0',
                .channel = 0,
                .listen_interval = 0,
                .pmf_cfg = {
                    .capable = true,
                    .required = false},
            },
        };
        memcpy(sta_config.sta.ssid, ssid, strlen(ssid));
        memcpy(sta_config.sta.password, password, strlen(password));

        status = esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config);
        if (status != ESP_OK)
        {
            LOGE(TAG, "set wifi sta configs failed");
            break;
        }

        status = esp_wifi_start();
        if (status != ESP_OK)
        {
            LOGE(TAG, "start wifi failed");
            break;
        }

        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                               WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                               pdFALSE,
                                               pdFALSE,
                                               portMAX_DELAY);

        if (bits & WIFI_CONNECTED_BIT)
        {
            LOGI(TAG, "start wlanSTA completed. connected to ssid:%s password:%s", ssid, password);
            status = ESP_OK;
        }
        else
        {
            LOGI(TAG, "failed to connect to ssid:%s password:%s", ssid, password);
            status = ESP_FAIL;

            StopWlanStation();
            DeinitializeWifi();
        }

    } while (false);

    return status;
}

esp_err_t StopWlanStation()
{
    esp_err_t status = ESP_OK;

    do
    {
        status = esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &WlanStationEventHandler);
        if (status != ESP_OK)
        {
            LOGE(TAG, "unregister wifi event failed");
            break;
        }

        status = esp_event_handler_unregister(IP_EVENT, ESP_EVENT_ANY_ID, &WlanStationEventHandler);
        if (status != ESP_OK)
        {
            LOGE(TAG, "unregister IP event failed");
            break;
        }

        status = esp_event_loop_delete_default();
        if (status != ESP_OK)
        {
            LOGE(TAG, "delete wifi event loop failed");
            break;
        }

        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        vEventGroupDelete(s_wifi_event_group);

        status = esp_wifi_disconnect();
        if (status != ESP_OK)
        {
            LOGE(TAG, "disconnect wifi failed");
            break;
        }

        status = esp_wifi_stop();
        if (status != ESP_OK)
        {
            LOGE(TAG, "stop wifi failed");
            break;
        }

    } while (false);

    return status;
}