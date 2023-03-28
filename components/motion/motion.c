#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#ifdef CONFIG_ENABLE_MOTION_DETECTION

#include "motion.h"

#define MOTION_DETECTION_PIN (CONFIG_MOTION_DETECTION_PIN)

motion_event_cb_t motion_event_cb = NULL;
static QueueHandle_t gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void motion_detection_task(void *arg)
{
    uint32_t gpio_num;
    for (;;)
    {
        if (xQueueReceive(gpio_evt_queue, &gpio_num, portMAX_DELAY))
        {
            if (gpio_num == MOTION_DETECTION_PIN && motion_event_cb != NULL)
            {
                motion_event_cb(true);
            }
        }
    }
}

void motion_register_callback(motion_event_cb_t callback)
{
    motion_event_cb = callback;
}

void motion_sensor_unregister_callback()
{
    motion_event_cb = NULL;
}

void motion_init()
{
    gpio_config_t io_config = {
        .intr_type = GPIO_INTR_POSEDGE,                 // rising edge intr
        .pin_bit_mask = (1ULL << MOTION_DETECTION_PIN), // bit mask of the input pin
        .mode = GPIO_MODE_INPUT,                        // set as input mode
        .pull_up_en = 1,                                // enable pull-up mode
        .pull_down_en = 0,                              // disable pull-down mode
    };

    gpio_config(&io_config);

    // create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    // start teach listener task
    xTaskCreate(motion_detection_task, "motion_detection_task", 2048, NULL, 1, NULL);

    // install gpio isr service default flag
    gpio_install_isr_service(0);
    // hook isr handler for specific gpio pin
    gpio_isr_handler_add(MOTION_DETECTION_PIN, gpio_isr_handler, (void *)MOTION_DETECTION_PIN);
}

#endif /* CONFIG_ENABLE_MOTION_DETECTION */