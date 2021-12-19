#include "esp_wifi.h"
#include "esp_err.h"

#include "Nvs.h"
#include "Logging.h"

#define SSID_SIZE (32)
#define PASSWORD_SIZE (64)

typedef struct
{
    char ssid[SSID_SIZE];
    char password[PASSWORD_SIZE];
    esp_netif_ip_info_t ipInfo;
} connection_info_t;

void DumpConnectionInfo(connection_info_t *pConnectionInfo);

esp_err_t InitializeWifi(void);
esp_err_t DeinitializeWifi(void);

esp_err_t SetHostname(esp_netif_t *esp_netif, const char *hostname);
esp_err_t GetHostname(esp_netif_t *esp_netif, const char **hostname);

esp_err_t SetIpInfo(esp_netif_t *esp_netif, esp_netif_ip_info_t *ip_info);
esp_err_t GetIpInfo(esp_netif_t *esp_netif, esp_netif_ip_info_t *ip_info);
