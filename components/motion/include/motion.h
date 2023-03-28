#ifndef __MOTION_H__
#define __MOTION_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef CONFIG_ENABLE_MOTION_DETECTION

    /**
     * @brief motion detection callback function type
     * @param event     : Event type
     * @param motion    : Point to callback value
     **/
    typedef void (*motion_event_cb_t)(bool motion);

    /**
     * @brief           This function is called to register a motion detection event
     *
     * @param[in]       callback: callback function
     */
    void motion_register_callback(motion_event_cb_t callback);

    /**
     * @brief           This function is called to unregister a motion detection  event
     */
    void motion_unregister_callback();

    /**
     * @brief           This function is called to initialize the motion detection event
     * @note            It's recommended to call motion_register_callback(motion_event_cb_t callback) before calling this function
     */
    void motion_init();

#endif /* CONFIG_ENABLE_MOTION_DETECTION */

#ifdef __cplusplus
}
#endif

#endif /* __MOTION_H__ */