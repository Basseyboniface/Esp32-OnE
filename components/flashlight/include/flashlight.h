#ifndef __FLASHLIGHT_H__
#define __FLASHLIGHT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef CONFIG_ENABLE_FLASHLIGHT

    /**
     * @brief   This function is called to initialize the camera flashlight
     */
    void flashlight_init();

    /**
     * @brief   This function is called to deinitialize the camera flashlight
     */
    void flashlight_deinit();

    /**
     * @brief   Function to set the camera flashlight intensity
     */
    void flashlight_intensity(uint8_t value);

#endif /* CONFIG_ENABLE_FLASHLIGHT */

#ifdef __cplusplus
}
#endif

#endif /* __FLASHLIGHT_H__ */
