#ifndef __LOGGING_H__
#define __LOGGING_H__

#define ENABLE_LOGGING
#ifdef ENABLE_LOGGING
#include "esp_log.h"

#define LOGI(TAG, str, ...) ESP_LOGI(TAG, str, ##__VA_ARGS__)
#define LOGW(TAG, str, ...) ESP_LOGW(TAG, str, ##__VA_ARGS__)
#define LOGE(TAG, str, ...) ESP_LOGE(TAG, str, ##__VA_ARGS__)
#else
#define LOGI(TAG, str, ...)
#define LOGW(TAG, str, ...)
#define LOGE(TAG, str, ...)
#endif

#endif /* __LOGGING_H__ */