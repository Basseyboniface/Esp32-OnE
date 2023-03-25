#include <stdio.h>
#include <esp_camera.h>

#include "camera.h"
#include "camera_defs.h"
#include "flashlight.h"

static void camera_reset()
{
    sensor_t *sensor = esp_camera_sensor_get();
    if (sensor != NULL)
    {
        sensor->set_framesize(sensor, FRAMESIZE_SVGA);
        sensor->set_quality(sensor, 14);
        sensor->set_brightness(sensor, 0);
        sensor->set_contrast(sensor, 0);
        sensor->set_saturation(sensor, 0);
        sensor->set_sharpness(sensor, 0);
        sensor->set_denoise(sensor, 1);
        sensor->set_special_effect(sensor, 0);
        sensor->set_wb_mode(sensor, 0);
        sensor->set_whitebal(sensor, 1);
        sensor->set_awb_gain(sensor, 1);
        sensor->set_exposure_ctrl(sensor, 1);
        sensor->set_aec2(sensor, 1);
        sensor->set_ae_level(sensor, 1);
        sensor->set_aec_value(sensor, 600);
        sensor->set_gain_ctrl(sensor, 1);
        sensor->set_agc_gain(sensor, 15);
        sensor->set_gainceiling(sensor, GAINCEILING_16X);
        sensor->set_bpc(sensor, 1);
        sensor->set_wpc(sensor, 1);
        sensor->set_raw_gma(sensor, 0);
        sensor->set_lenc(sensor, 1);
        sensor->set_hmirror(sensor, 0);
        sensor->set_vflip(sensor, 0);
        sensor->set_dcw(sensor, 0);
        sensor->set_colorbar(sensor, 0);
    }
}

esp_err_t camera_init()
{
    camera_config_t cfg;

    cfg.pin_pwdn = PWDN_GPIO_NUM;
    cfg.pin_reset = RESET_GPIO_NUM;
    cfg.pin_xclk = XCLK_GPIO_NUM;
    cfg.pin_sccb_sda = SIOD_GPIO_NUM;
    cfg.pin_sccb_scl = SIOC_GPIO_NUM;

    cfg.pin_d7 = Y9_GPIO_NUM;
    cfg.pin_d6 = Y8_GPIO_NUM;
    cfg.pin_d5 = Y7_GPIO_NUM;
    cfg.pin_d4 = Y6_GPIO_NUM;
    cfg.pin_d3 = Y5_GPIO_NUM;
    cfg.pin_d2 = Y4_GPIO_NUM;
    cfg.pin_d1 = Y3_GPIO_NUM;
    cfg.pin_d0 = Y2_GPIO_NUM;
    cfg.pin_vsync = VSYNC_GPIO_NUM;
    cfg.pin_href = HREF_GPIO_NUM;
    cfg.pin_pclk = PCLK_GPIO_NUM;

    cfg.xclk_freq_hz = 20000000;
    cfg.ledc_timer = LEDC_TIMER_0;
    cfg.ledc_channel = LEDC_CHANNEL_0;
    cfg.pixel_format = PIXFORMAT_JPEG; // YUV422,GRAYSCALE,RGB565,JPEG

    cfg.frame_size = FRAMESIZE_SVGA; // QQVGA-UXGA Do not use sizes above QVGA when not JPEG
    cfg.jpeg_quality = 12;           // 0-63 lower number means higher quality
    cfg.fb_count = 1;                // if more than one, i2s runs in continuous mode. Use only with JPEG
    cfg.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    cfg.fb_location = CAMERA_FB_IN_DRAM;
#ifdef CONFIG_ESP32_SPIRAM_SUPPORT
    cfg.frame_size = FRAMESIZE_UXGA; // QQVGA-UXGA Do not use sizes above QVGA when not JPEG
    cfg.jpeg_quality = 10;           // 0-63 lower number means higher quality
    cfg.fb_count = 2;                // if more than one, i2s runs in continuous mode. Use only with JPEG
    cfg.grab_mode = CAMERA_GRAB_LATEST;
    cfg.fb_location = CAMERA_FB_IN_PSRAM;
#endif

    esp_err_t ret = esp_camera_init(&cfg);
    if (ret == ESP_OK)
    {
        camera_reset();

#ifdef CONFIG_ENABLE_FLASHLIGHT
        flashlight_init();
#endif
    }
    return ret;
}

void camera_deinit()
{
#ifdef CONFIG_ENABLE_FLASHLIGHT
    flashlight_deinit();
#endif
}
