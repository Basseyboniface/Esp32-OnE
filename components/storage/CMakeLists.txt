idf_component_register(SRCS "credential.c" "sdcard.c"
                    INCLUDE_DIRS "include"
                    REQUIRES fatfs vfs sdmmc esp32-camera esp_timer nvs_flash esp_wifi)
