#include <stdio.h>
#include "esp_err.h"
#include "app.h"

#include "wiFi_softAP.h"
#include "webserver.h"

void app_start(void)
{
    esp_err_t ret = wifi_init_SOFTAP_mode("camOne WiFi", "password");
    if (ret == ESP_OK)
        printf("start SOFTAP: %d\n", ret);

    ret = webserver_start();
    if (ret == ESP_OK)
        printf("start Webserver: %d\n", ret);
}
