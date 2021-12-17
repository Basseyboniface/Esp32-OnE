#include "esp_wifi.h"
#include "esp_err.h"

#include "Nvs.h"
#include "Logging.h"

esp_err_t InitializeWifi(void);
esp_err_t DeinitializeWifi(void);

esp_err_t SetHostname(esp_netif_t *esp_netif, const char *hostname);
esp_err_t GetHostname(esp_netif_t *esp_netif, const char **hostname);
