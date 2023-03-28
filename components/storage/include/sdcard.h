#ifndef __SDCARD_H__
#define __SDCARD_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "esp_err.h"

    esp_err_t sdcard_init();

    esp_err_t sdcard_saveImage(unsigned char *buffer, size_t size);

    void sdcard_deleteImage(const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* __SDCARD_H__ */