#include <stdio.h>
#include "Nvs.h"

esp_err_t InitializeNvs(void)
{
    esp_err_t status = nvs_flash_init();
    if (status == ESP_ERR_NVS_NO_FREE_PAGES || status == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      status = nvs_flash_init();
    }

    return status;
}
