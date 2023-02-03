#ifndef INCLUDED_SDCARD_H_
#define INCLUDED_SDCARD_H_

#include "esp_err.h"

esp_err_t sdcard_init();

esp_err_t sdcard_saveImage(unsigned char *buffer, size_t size);

void sdcard_deleteImage(const char *filename);

#endif /* INCLUDED_SDCARD_H_ */