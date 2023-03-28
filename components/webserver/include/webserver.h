#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

#ifdef __cplusplus
extern "C"
{
#endif

    esp_err_t webserver_start();

    esp_err_t webserver_stop();

#ifdef __cplusplus
}
#endif

#endif /* __WEBSERVER_H__ */
