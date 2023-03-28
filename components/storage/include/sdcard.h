#ifndef __SDCARD_H__
#define __SDCARD_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "esp_err.h"

    esp_err_t onecam_sdcard_init();

    esp_err_t onecam_save_image(unsigned char *buffer, size_t size);

    void onecam_delete_image(const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* __SDCARD_H__ */