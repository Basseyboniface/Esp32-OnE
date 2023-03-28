#include <string.h>
#include <stdint.h>

#include <nvs_flash.h>
#include "esp_log.h"
#include "credential.h"

static const char *NAMESPACE = "namespace";
static const char *KEY = "key";
static const char *VERSION = "version";
static uint32_t version = 0x0100;

static const char *TAG = "credentials";

esp_err_t store_credentials(credential_t *data)
{
    esp_err_t ret = ESP_OK;

    do
    {
        nvs_handle_t handle;
        ret = nvs_open(NAMESPACE, NVS_READWRITE, &handle);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "open non-volatile storage failed. status=%d, handle=%lu", ret, handle);
            break;
        }

        size_t sizeRequired = 0;
        ret = nvs_get_blob(handle, KEY, NULL, &sizeRequired);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "nvs get value for given key failed. status=%d", ret);
            break;
        }

        size_t dataSize = sizeof(credential_t);
        ESP_LOGI(TAG, "Required size: %d, Data size: %d", sizeRequired, dataSize);

        ret = nvs_set_blob(handle, KEY, data, dataSize);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "nvs set value for given key failed. status=%d", ret);
            break;
        }

        ret = nvs_set_u32(handle, VERSION, version);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "set version failed. status=%d", ret);
            break;
        }

        ret = nvs_commit(handle);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "nvs commit failed. status=%d", ret);
            break;
        }

        nvs_close(handle);

    } while (false);

    return ret;
}

esp_err_t retrieve_credentials(credential_t *data)
{
    esp_err_t ret = ESP_OK;

    do
    {
        nvs_handle_t handle;
        ret = nvs_open(NAMESPACE, NVS_READWRITE, &handle);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "open non-volatile storage failed. status=%d, handle=%lu", ret, handle);
            break;
        }

        uint32_t m_version = 0;
        ret = nvs_get_u32(handle, VERSION, &m_version);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "get version failed. status=%d", ret);
            break;
        }

        // Check the versions match
        if ((m_version & 0xff00) != (version & 0xff00))
        {
            ESP_LOGE(TAG, "incompatible versions ... current is %lu, found is %lu", m_version, version);
            return ESP_FAIL;
        }

        size_t dataSize = sizeof(credential_t);
        ret = nvs_get_blob(handle, KEY, data, &dataSize);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "nvs get value for given key failed. status=%d", ret);
            break;
        }

        if (strlen(data->ssid) == 0)
        {
            ESP_LOGE(TAG, "No connection information found.");
            return ESP_FAIL;
        }

        nvs_close(handle);

    } while (false);

    return ret;
}