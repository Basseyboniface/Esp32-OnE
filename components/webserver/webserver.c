#include <string.h>
#include <stdio.h>
#include <esp_http_server.h>
#include <esp_camera.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_timer.h>

#include "webserver.h"
#include "camera_defs.h"
#include "flashlight.h"
#include "camera_html_gz.h"
#include "onecam_motion.h"

static const char *TAG = "oneCam-Webserver";

httpd_handle_t stream_handle = NULL;
httpd_handle_t snapshot_handle = NULL;

static uint8_t ledc_value = 0;
bool streaming = false;

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

typedef struct
{
    httpd_req_t *req;
    size_t len;
} jpg_chunking_t;

static size_t jpg_encode_stream(void *arg, size_t index, const void *data, size_t len)
{
    jpg_chunking_t *j = (jpg_chunking_t *)arg;
    if (!index)
    {
        j->len = 0;
    }
    if (httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK)
    {
        return 0;
    }
    j->len += len;
    return len;
}

static esp_err_t index_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    return httpd_resp_send(req, (const char *)camera_html_gz, camera_html_gz_len);
}

static esp_err_t control_handler(httpd_req_t *req)
{
    size_t buf_len = httpd_req_get_url_query_len(req) + 1;
    if (!(buf_len > 1))
    {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    esp_err_t query_url, query_var, query_val;
    char variable[32] = {0};
    char value[32] = {0};

    char *buf = (char *)malloc(buf_len);
    if (!buf)
    {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    query_url = httpd_req_get_url_query_str(req, buf, buf_len);
    if (query_url == ESP_OK)
    {
        query_var = httpd_query_key_value(buf, "var", variable, sizeof(variable));
        query_val = httpd_query_key_value(buf, "val", value, sizeof(value));
    }

    free(buf);
    if (query_url != ESP_OK || (query_var != ESP_OK && query_val != ESP_OK))
    {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    sensor_t *sensor = esp_camera_sensor_get();
    int val = atoi(value);
    int result = 0;

    if (!strcmp(variable, "framesize"))
    {
        if (sensor->pixformat == PIXFORMAT_JPEG)
            result = sensor->set_framesize(sensor, (framesize_t)val);
    }
    else if (!strcmp(variable, "quality"))
        result = sensor->set_quality(sensor, val);
    else if (!strcmp(variable, "contrast"))
        result = sensor->set_contrast(sensor, val);
    else if (!strcmp(variable, "brightness"))
        result = sensor->set_brightness(sensor, val);
    else if (!strcmp(variable, "saturation"))
        result = sensor->set_saturation(sensor, val);
    else if (!strcmp(variable, "gainceiling"))
        result = sensor->set_gainceiling(sensor, (gainceiling_t)val);
    else if (!strcmp(variable, "colorbar"))
        result = sensor->set_colorbar(sensor, val);
    else if (!strcmp(variable, "awb"))
        result = sensor->set_whitebal(sensor, val);
    else if (!strcmp(variable, "agc"))
        result = sensor->set_gain_ctrl(sensor, val);
    else if (!strcmp(variable, "aec"))
        result = sensor->set_exposure_ctrl(sensor, val);
    else if (!strcmp(variable, "hmirror"))
        result = sensor->set_hmirror(sensor, val);
    else if (!strcmp(variable, "vflip"))
        result = sensor->set_vflip(sensor, val);
    else if (!strcmp(variable, "awb_gain"))
        result = sensor->set_awb_gain(sensor, val);
    else if (!strcmp(variable, "agc_gain"))
        result = sensor->set_agc_gain(sensor, val);
    else if (!strcmp(variable, "aec_value"))
        result = sensor->set_aec_value(sensor, val);
    else if (!strcmp(variable, "aec2"))
        result = sensor->set_aec2(sensor, val);
    else if (!strcmp(variable, "dcw"))
        result = sensor->set_dcw(sensor, val);
    else if (!strcmp(variable, "bpc"))
        result = sensor->set_bpc(sensor, val);
    else if (!strcmp(variable, "wpc"))
        result = sensor->set_wpc(sensor, val);
    else if (!strcmp(variable, "raw_gma"))
        result = sensor->set_raw_gma(sensor, val);
    else if (!strcmp(variable, "lenc"))
        result = sensor->set_lenc(sensor, val);
    else if (!strcmp(variable, "special_effect"))
        result = sensor->set_special_effect(sensor, val);
    else if (!strcmp(variable, "wb_mode"))
        result = sensor->set_wb_mode(sensor, val);
    else if (!strcmp(variable, "ae_level"))
        result = sensor->set_ae_level(sensor, val);
#ifdef CONFIG_ONECAM_ENABLE_CAM_FLASH
    else if (!strcmp(variable, "lamp"))
    {
        ledc_value = val;
        if (streaming)
            flashlight_intensity(ledc_value);
    }
#endif
    else
        result = -1;

    if (result)
        return httpd_resp_send_500(req);

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}

static esp_err_t status_handler(httpd_req_t *req)
{
    sensor_t *sensor = esp_camera_sensor_get();
    if (sensor == NULL)
    {
        ESP_LOGE(TAG, "sensor is null");
        return ESP_FAIL;
    }

    static char json_response[1024];
    char *p = json_response;
    *p++ = '{';
    p += sprintf(p, "\"framesize\":%u,", sensor->status.framesize);
    p += sprintf(p, "\"quality\":%u,", sensor->status.quality);
    p += sprintf(p, "\"brightness\":%d,", sensor->status.brightness);
    p += sprintf(p, "\"contrast\":%d,", sensor->status.contrast);
    p += sprintf(p, "\"saturation\":%d,", sensor->status.saturation);
    p += sprintf(p, "\"sharpness\":%d,", sensor->status.sharpness);
    p += sprintf(p, "\"special_effect\":%u,", sensor->status.special_effect);
    p += sprintf(p, "\"wb_mode\":%u,", sensor->status.wb_mode);
    p += sprintf(p, "\"awb\":%u,", sensor->status.awb);
    p += sprintf(p, "\"awb_gain\":%u,", sensor->status.awb_gain);
    p += sprintf(p, "\"aec\":%u,", sensor->status.aec);
    p += sprintf(p, "\"aec2\":%u,", sensor->status.aec2);
    p += sprintf(p, "\"ae_level\":%d,", sensor->status.ae_level);
    p += sprintf(p, "\"aec_value\":%u,", sensor->status.aec_value);
    p += sprintf(p, "\"agc\":%u,", sensor->status.agc);
    p += sprintf(p, "\"agc_gain\":%u,", sensor->status.agc_gain);
    p += sprintf(p, "\"gainceiling\":%u,", sensor->status.gainceiling);
    p += sprintf(p, "\"bpc\":%u,", sensor->status.bpc);
    p += sprintf(p, "\"wpc\":%u,", sensor->status.wpc);
    p += sprintf(p, "\"raw_gma\":%u,", sensor->status.raw_gma);
    p += sprintf(p, "\"lenc\":%u,", sensor->status.lenc);
    p += sprintf(p, "\"vflip\":%u,", sensor->status.vflip);
    p += sprintf(p, "\"hmirror\":%u,", sensor->status.hmirror);
    p += sprintf(p, "\"dcw\":%u,", sensor->status.dcw);
    p += sprintf(p, "\"colorbar\":%u,", sensor->status.colorbar);
#ifdef CONFIG_ONECAM_ENABLE_CAM_FLASH
    p += sprintf(p, ",\"lamp\":%u", ledc_value);
#else
    p += sprintf(p, ",\"lamp\":%d", 0);
#endif
    *p++ = '}';
    *p++ = 0;

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, json_response, strlen(json_response));
}

static camera_fb_t *capture_image()
{
    camera_fb_t *fb = NULL;

#ifdef CONFIG_ONECAM_ENABLE_CAM_FLASH
    flashlight_intensity(ledc_value);
    vTaskDelay(150 / portTICK_PERIOD_MS); // The LED requires ~150ms to "warm up"
#endif

    fb = esp_camera_fb_get();

#ifdef CONFIG_ONECAM_ENABLE_CAM_FLASH
    flashlight_intensity(0);
#endif

    return fb;
}

static esp_err_t capture_handler(httpd_req_t *req)
{
    // camera_fb_t *fb = NULL;
    esp_err_t ret = ESP_OK;
    int64_t frame_start = esp_timer_get_time();

    // #ifdef CONFIG_ONECAM_ENABLE_CAM_FLASH
    //     flashlight_intensity(ledc_value);
    //     vTaskDelay(150 / portTICK_PERIOD_MS); // The LED requires ~150ms to "warm up"
    // #endif
    //
    //     fb = esp_camera_fb_get();
    //
    // #ifdef CONFIG_ONECAM_ENABLE_CAM_FLASH
    //     flashlight_intensity(0);
    // #endif

    camera_fb_t *fb = capture_image();
    if (!fb)
    {
        ESP_LOGE(TAG, "camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");

    size_t fb_len = 0;
    if (fb->format == PIXFORMAT_JPEG)
    {
        fb_len = fb->len;
        ret = httpd_resp_send(req, (const char *)fb->buf, fb->len);
    }
    else
    {
        jpg_chunking_t jchunk = {req, 0};
        ret = frame2jpg_cb(fb, 80, jpg_encode_stream, &jchunk) ? ESP_OK : ESP_FAIL;
        httpd_resp_send_chunk(req, NULL, 0);
        fb_len = jchunk.len;
    }

    esp_camera_fb_return(fb);
    int64_t frame_end = esp_timer_get_time();
    ESP_LOGI(TAG, "JPG: %luKB %lums", (uint32_t)(fb_len / 1024), (uint32_t)((frame_end - frame_start) / 1000));

    return ret;
}

static esp_err_t stream_handler(httpd_req_t *req)
{
    esp_err_t ret = ESP_OK;

    camera_fb_t *fb = NULL;

    size_t _jpg_buf_len;
    uint8_t *_jpg_buf;
    char *part_buf[64];

    static int64_t last_frame = 0;
    if (!last_frame)
        last_frame = esp_timer_get_time();

    ret = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (ret != ESP_OK)
        return ESP_FAIL;

#ifdef CONFIG_ONECAM_ENABLE_CAM_FLASH
    flashlight_intensity(ledc_value);
#endif

    streaming = true;

    while (true)
    {
        fb = esp_camera_fb_get();
        if (!fb)
        {
            ESP_LOGE(TAG, "Camera capture failed");
            ret = ESP_FAIL;
            break;
        }

        if (fb->format != PIXFORMAT_JPEG)
        {
            bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
            if (!jpeg_converted)
            {
                ESP_LOGE(TAG, "JPEG compression failed");
                esp_camera_fb_return(fb);
                ret = ESP_FAIL;
            }
        }
        else
        {
            _jpg_buf_len = fb->len;
            _jpg_buf = fb->buf;
        }

        if (ret == ESP_OK)
        {
            ret = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
            if (ret == ESP_OK)
            {
                size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);

                ret = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
                if (ret == ESP_OK)
                    ret = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
            }
        }

        if (fb->format != PIXFORMAT_JPEG)
            free(_jpg_buf);

        esp_camera_fb_return(fb);
        if (ret != ESP_OK)
            break;

        last_frame = esp_timer_get_time();
        // int64_t frame_end = esp_timer_get_time();
        // int64_t frame_time = frame_end - last_frame;
        // last_frame = frame_end;
        // frame_time /= 1000;
        // ESP_LOGI(TAG, "MJPG: %luKB %lums (%.1ffps)",
        //          (uint32_t)(_jpg_buf_len / 1024),
        //          (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time);
    }

#ifdef CONFIG_ONECAM_ENABLE_CAM_FLASH
    flashlight_intensity(0);
#endif

    last_frame = 0;
    streaming = false;
    return ret;
}

void callback_on_motion_detected(bool motion)
{
    printf("motion detected: %d", motion);
    // camera_fb_t *fb = capture_image();
    // if (!fb)
    //{
    //     esp_err_t ret = onecam_save_image(fb->buf, fb->len)
    // }
}

esp_err_t webserver_start()
{
    // Initialize the camera
    esp_err_t ret = camera_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "camera initialization failed");
        return ret;
    }

    // Initialize the onecam motion detection
    onecam_pir_register_callback(callback_on_motion_detected);
    onecam_pir_init();

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_uri_t index_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = index_handler,
        .user_ctx = NULL,
    };

    httpd_uri_t control_uri = {
        .uri = "/control",
        .method = HTTP_GET,
        .handler = control_handler,
        .user_ctx = NULL,
    };

    httpd_uri_t status_uri = {
        .uri = "/status",
        .method = HTTP_GET,
        .handler = status_handler,
        .user_ctx = NULL,
    };

    httpd_uri_t capture_uri = {
        .uri = "/capture",
        .method = HTTP_GET,
        .handler = capture_handler,
        .user_ctx = NULL,
    };

    httpd_uri_t stream_uri = {
        .uri = "/stream",
        .method = HTTP_GET,
        .handler = stream_handler,
        .user_ctx = NULL,
    };

    ESP_LOGI(TAG, "starting capture-server on port: '%d'", config.server_port);
    esp_err_t result = httpd_start(&snapshot_handle, &config);
    if (result != ESP_OK)
    {
        snapshot_handle = NULL;
        return result;
    }

    httpd_register_uri_handler(snapshot_handle, &index_uri);
    httpd_register_uri_handler(snapshot_handle, &control_uri);
    httpd_register_uri_handler(snapshot_handle, &status_uri);
    httpd_register_uri_handler(snapshot_handle, &capture_uri);

    config.server_port += 1;
    config.ctrl_port += 1;
    ESP_LOGI(TAG, "starting stream-server on port: '%d'", config.server_port);
    result = httpd_start(&stream_handle, &config);
    if (result == ESP_OK)
    {
        httpd_register_uri_handler(stream_handle, &stream_uri);
    }

    return result;
}

esp_err_t webserver_stop()
{
    esp_err_t ret = ESP_FAIL;

    if (stream_handle)
    {
        ret = httpd_stop(stream_handle);
    }

    if (snapshot_handle)
    {
        ret = httpd_stop(snapshot_handle);
    }

    return ret;
}