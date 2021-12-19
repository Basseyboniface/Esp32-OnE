#include <stdio.h>
#include "WlanSoftAP.h"
#include "WlanStation.h"

void app_main(void)
{
    StartWlanSoftAP("Esp32-OnE", "password");
    //StartWlanStation("your-ssid", "your-passowrd");
}
