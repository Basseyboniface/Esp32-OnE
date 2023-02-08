#ifndef __CREDENTIALS_H__
#define __CREDENTIALS_H__

#ifdef __cplusplus
extern "C"
{
#endif

//------------------------------------------
// Includes
//------------------------------------------
#include "esp_err.h"
#include "esp_wifi.h"

#define SIZE_32 32
#define SIZE_64 64

    typedef struct
    {
        char ssid[SIZE_32];
        char password[SIZE_64];
        esp_netif_ip_info_t ip_info;
    } credential_t;

    //------------------------------------------
    // Prototypes
    //------------------------------------------

    /**
     * @brief   Stores connection credentials to nvs-flash
     *
     * @param  data: Data to be stored
     */
    esp_err_t store_credentials(credential_t *data);

    /**
     * @brief   Retrieves connection credentials from nvs-flash
     *
     * @param  data: Retrieved data
     */
    esp_err_t retrieve_credentials(credential_t *data);

#ifdef __cplusplus
}
#endif

#endif /* __CREDENTIALS_H__ */