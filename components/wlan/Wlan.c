#include <stdio.h>
#include "Wlan.h"

static const char *TAG = "Wlan";

esp_err_t InitializeWifi(void)
{
    esp_err_t status = ESP_OK;
    do
    {
        status = InitializeNvs();
        if (status != ESP_OK)
        {
            LOGE(TAG, "initialize nvs flash failed");
            break;
        }

        status = esp_netif_init();
        if (status != ESP_OK)
        {
            LOGE(TAG, "init netif failed");
            break;
        }

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        status = esp_wifi_init(&cfg);
        if (status != ESP_OK)
        {
            LOGE(TAG, "initialize default config failed");
            break;
        }

        status = esp_event_loop_create_default();
        if (status != ESP_OK)
        {
            LOGE(TAG, "create event loop failed");
            break;
        }

    } while (false);

    return status;
}

esp_err_t DeinitializeWifi(void)
{
    esp_err_t status = ESP_OK;

    do
    {
        status = esp_wifi_deinit();
        if (status != ESP_OK)
            break;

        //Note: Deinitialization is not supported yet
        //status = esp_netif_deinit();
        //if (status != ESP_OK)
        //    break;

    } while (false);

    return status;
}

esp_err_t SetHostname(esp_netif_t *esp_netif, const char *hostname)
{
    return esp_netif_set_hostname(esp_netif, hostname);
}

esp_err_t GetHostname(esp_netif_t *esp_netif, const char **hostname)
{
    const char *_hostname;
    esp_err_t ret = esp_netif_get_hostname(esp_netif, &_hostname);

    *hostname = _hostname;

    return ret;
}
