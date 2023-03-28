idf_component_register(SRCS "webserver.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "esp_http_server" "esp32-camera" "camera" "flashlight" "esp_timer")
