idf_component_register(SRCS "sdcard.c"
                    INCLUDE_DIRS "include"
                    REQUIRES fatfs vfs sdmmc esp32-camera esp_timer)
