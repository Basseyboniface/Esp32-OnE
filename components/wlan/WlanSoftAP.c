#include <string.h>

#include "Wlan.h"
#include "WlanSoftAP.h"

static const char *TAG = "WlanSoftAP";

esp_event_handler_instance_t event_instance;
static esp_netif_t *m_wlan = NULL;

static void WlanSoftAPEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_AP_START)
        LOGI(TAG, "WIFI_EVENT_AP_START");

    else if (event_id == WIFI_EVENT_AP_STOP)
        LOGI(TAG, "WIFI_EVENT_AP_START");

    else if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
    }

    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        LOGI(TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
}

esp_err_t StartWlanSoftAP(const char *ssid, const char *password)
{
    esp_err_t status = ESP_OK;
    do
    {
        status = InitializeWifi();
        if (status != ESP_OK)
        {
            LOGE(TAG, "initialize wifi failed");
            break;
        }

        status = esp_event_loop_create_default();
        if (status != ESP_OK)
        {
            LOGE(TAG, "create event loop failed");
            break;
        }

        m_wlan = esp_netif_create_default_wifi_ap();

        status = esp_event_handler_instance_register(WIFI_EVENT,
                                                     ESP_EVENT_ANY_ID,
                                                     &WlanSoftAPEventHandler,
                                                     NULL,
                                                     &event_instance);
        if (status != ESP_OK)
        {
            LOGE(TAG, "register event handler failed");
            break;
        }

        status = esp_wifi_set_mode(WIFI_MODE_AP);
        if (status != ESP_OK)
        {
            LOGE(TAG, "set wifi AP mode failed");
            break;
        }

        wifi_config_t ap_config = {
            .ap = {
                .ssid_len = strlen(ssid),
                .channel = 1,
                .max_connection = 4,
                .beacon_interval = 100,
                .authmode = WIFI_AUTH_WPA_WPA2_PSK,
                .ssid_hidden = 0,
            },
        };
        memcpy(ap_config.ap.ssid, ssid, strlen(ssid));
        memcpy(ap_config.ap.password, password, strlen(password));
        if (strlen(password) == 0)
            ap_config.ap.authmode = WIFI_AUTH_OPEN;

        status = esp_wifi_set_config(WIFI_IF_AP, &ap_config);
        if (status != ESP_OK)
        {
            LOGE(TAG, "set wifi ap configs failed");
            break;
        }

        status = esp_wifi_start();
        if (status != ESP_OK)
        {
            LOGE(TAG, "start wifi failed");
            break;
        }

        LOGI(TAG, "start wlan-soft-ap successful. ssid:%s password:%s", ssid, password);

    } while (false);

    return status;
}

esp_err_t StopWlanSoftAP()
{
    esp_err_t status = ESP_OK;
    do
    {
        status = esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, event_instance);
        if (status != ESP_OK)
        {
            LOGE(TAG, "unregister event handler failed");
            break;
        }
        status = esp_event_loop_delete_default();
        if (status != ESP_OK)
        {
            LOGE(TAG, "delete event loop failed");
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