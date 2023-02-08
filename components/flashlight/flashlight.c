#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "driver/ledc.h"
#include "esp_log.h"

#include "flashlight.h"

#ifdef CONFIG_ENABLE_FLASHLIGHT

#ifdef CONFIG_LEDC_LOW_SPEED_MODE
#define CONFIG_LEDC_SPEED_MODE (LEDC_LOW_SPEED_MODE)
#else
#define CONFIG_LEDC_SPEED_MODE (LEDC_HIGH_SPEED_MODE)
#endif

static const char *TAG = "flashlight";

void flashlight_init()
{
    gpio_set_direction(CONFIG_LEDC_PIN, GPIO_MODE_OUTPUT);

    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_8_BIT, // resolution of PWM duty
        .freq_hz = 1000,                     // frequency of PWM signal
        .speed_mode = LEDC_LOW_SPEED_MODE,   // timer mode
        .timer_num = CONFIG_LEDC_TIMER       // timer index
    };

    ledc_channel_config_t ledc_channel = {
        .channel = CONFIG_LEDC_CHANNEL,
        .duty = 0,
        .gpio_num = CONFIG_LEDC_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = CONFIG_LEDC_TIMER,
    };

#ifdef CONFIG_LEDC_HIGH_SPEED_MODE
    ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;
    ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
#endif

    if (ledc_timer_config(&ledc_timer) == ESP_OK)
    {
        if (ledc_channel_config(&ledc_channel) != ESP_OK)
            ESP_LOGE(TAG, "configure camera flashlight failed");
    }
}

void flashlight_deinit()
{
    if (ledc_stop(CONFIG_LEDC_SPEED_MODE, CONFIG_LEDC_CHANNEL, 0) != ESP_OK)
    {
        ESP_LOGE(TAG, "stop camera flashlight failed");
    }
}

void flashlight_intensity(uint8_t value)
{
    uint8_t duty = (value >= CONFIG_LED_MAX_INTENSITY) ? CONFIG_LED_MAX_INTENSITY : value;
    ledc_set_duty(CONFIG_LEDC_SPEED_MODE, CONFIG_LEDC_CHANNEL, duty);
    ledc_update_duty(CONFIG_LEDC_SPEED_MODE, CONFIG_LEDC_CHANNEL);

    ESP_LOGI(TAG, "flashlight intensity set to: %d", duty);
}

#endif