#ifndef INCLUDED_SDCARD_H_
#define INCLUDED_SDCARD_H_

#include "esp_err.h"
#include "esp_camera.h"

esp_err_t sdcard_init();

esp_err_t sdcard_saveImage(camera_fb_t *fb);


#endif /* INCLUDED_SDCARD_H_ */