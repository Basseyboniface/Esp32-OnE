#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#ifdef CONFIG_ONECAM_ENABLE_PIR_MOTION

#include "onecam_motion.h"

onecam_pir_event_cb_t onecam_pir_event_cb = NULL;
onecam_pir_config_t *pir_config;

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(pir_config->gpio_evt_queue, &gpio_num, NULL);
}

static void onecam_motion_task(void *arg)
{
    uint32_t gpio_num;
    for (;;)
    {
        if (xQueueReceive(pir_config->gpio_evt_queue, &gpio_num, portMAX_DELAY))
        {
            if (gpio_num == pir_config->data_gpio_num && onecam_pir_event_cb != NULL)
            {
                onecam_pir_event_cb(true);
            }
        }
    }
}

void onecam_pir_init()
{
    if (pir_config->is_init)
        return;

    // WAIT 5 seconds after power-on so the PIR Sensor can calibrate itself!
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    onecam_pir_config_t *pir_config = &(onecam_pir_config_t)ONECAM_PIR_CONFIG_DEFAULT();
    pir_config->gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    gpio_config_t io_config = {
        .pin_bit_mask = (1ULL << pir_config->data_gpio_num), // bit mask of the input pin
        .mode = GPIO_MODE_INPUT,                             // set as input mode
        .pull_up_en = GPIO_PULLUP_ENABLE,                    // enable pull-up mode
        .pull_down_en = GPIO_PULLDOWN_DISABLE,               // disable pull-down mode
        .intr_type = GPIO_INTR_POSEDGE,                      // rising edge intr
    };
    gpio_config(&io_config);

    // ISR
    // ESP_INTR_FLAG_LEVEL1 Accept a Level 1 interrupt vector (lowest priority)
    esp_err_t ret = gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);
    if (ret != ESP_OK)
    {
    }
    // hook isr handler for specific gpio pin
    ret = gpio_isr_handler_add(pir_config->data_gpio_num, gpio_isr_handler, (void *)pir_config->data_gpio_num);
    if (ret != ESP_OK)
    {
    }

    // Flag state
    pir_config->is_init = true;

    // start motion detection task
    xTaskCreate(onecam_motion_task, "onecam_motion_task", 2048, NULL, 10, NULL);
}

void onecam_pir_register_callback(onecam_pir_event_cb_t callback)
{
    onecam_pir_event_cb = callback;
}

void onecam_pir_unregister_callback()
{
    onecam_pir_event_cb = NULL;
}

void onecam_pir_deinit()
{
    if (!pir_config->is_init)
        return;

    // ISR
    esp_err_t ret = gpio_isr_handler_remove(pir_config->data_gpio_num);
    if (ret != ESP_OK)
    {
    }

    gpio_uninstall_isr_service();

    // QueueHandle
    vQueueDelete(pir_config->gpio_evt_queue);
    pir_config->gpio_evt_queue = NULL;

    // Flag state
    pir_config->is_init = false;
}

#endif /* CONFIG_ONECAM_ENABLE_PIR_MOTION */