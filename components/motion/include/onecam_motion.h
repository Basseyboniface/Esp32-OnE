#ifndef __ONECAM_MOTION_H__
#define __ONECAM_MOTION_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef CONFIG_ONECAM_ENABLE_PIR_MOTION

    typedef struct
    {
        bool is_init;
        uint32_t data_gpio_num;
        QueueHandle_t gpio_evt_queue;
    } onecam_pir_config_t;

#define ONECAM_PIR_CONFIG_DEFAULT()             \
    {                                           \
        .is_init = false,                       \
        .data_gpio_num = CONFIG_ONECAM_PIR_PIN, \
        .gpio_evt_queue = NULL                  \
    }

    /**
     * @brief motion detection callback function type
     **/
    typedef void (*onecam_pir_event_cb_t)(bool is_triggered);

    /**
     * @brief       Registers the motion detection  event
     * @param[in]   callback: callback function
     */
    void onecam_pir_register_callback(onecam_pir_event_cb_t callback);

    /**
     * @brief       Unregisters the motion detection  event
     */
    void onecam_pir_unregister_callback();

    /**
     * @brief       Initializes the motion detection
     */
    void onecam_pir_init();

    /**
     * @brief       Deinitializes the motion detection
     */
    void onecam_pir_deinit();

#endif /* CONFIG_ONECAM_ENABLE_PIR_MOTION */

#ifdef __cplusplus
}
#endif

#endif /* __ONECAM_MOTION_H__ */