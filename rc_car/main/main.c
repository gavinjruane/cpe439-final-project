#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "motor_driver.h"
#include "bluetooth.h"
#include "ultrasonic.h"

#define GPIO_PIN_2 2

#define FORWARD "Forward\r\n"
#define BACKWARD "Backward\r\n"
#define LEFT "Left\r\n"
#define RIGHT "Right\r\n"
#define UNKNOWN "Unknown\r\n"

bool forward_movement = false;

hcsr04_t front_sensor;

void event_loop_task(void *argument);

TaskHandle_t event_loop_task_handle;

void app_main(void)
{
    // gpio_reset_pin(GPIO_PIN_2);

    // gpio_set_direction(GPIO_PIN_2, GPIO_MODE_OUTPUT);

    // int led_state = 0;
    // while (1) {
    //     gpio_set_level(GPIO_PIN_2, led_state);

    //     printf("LED is %s\n", led_state ? "ON" : "OFF");

    //     led_state = !led_state;

    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }

    BaseType_t result = pdFALSE;

    if (bt_init("ESP32_DEVICE") == -1)
    {
        printf("ERROR!!\n");
        return;
    }

    motor_driver_init();

    result = xTaskCreate(
        event_loop_task,
        "event_loop_task",
        8192,
        NULL,
        1,
        &event_loop_task_handle);
    if (result == pdFALSE)
    {
        printf("Could not create event_loop_task.\n");
        return;
    }

    if (hcsr04_init(&front_sensor, GPIO_NUM_13, GPIO_NUM_34) != ESP_OK)
    {
        printf("Failed to initialize HC-SR04\n");
        return;
    }

    while (1)
    {
        vTaskDelay(1);
    }
}

void event_loop_task(void *argument)
{
    struct bt_data data = {0};
    struct bt_data response = {0};
    extern volatile bool object_detected;
    // portMUX_TYPE myLock = portMUX_INITIALIZER_UNLOCKED;

    while (1)
    {
        memset(&response, 0, sizeof(struct bt_data));
        // if (ulTaskNotifyTake(pdTRUE, 0) >= 1)
        // {
        //     object_detected = true;
        //     motor_stop();
        //     strncpy(response.data, "Obstacle DETECTED!\r\n", sizeof(response.data));
        // } else {
        //     object_detected = false;
        // }

        // taskENTER_CRITICAL(&myLock);
        // if (object_detected && forward_movement) {
        //     motor_stop();
        //     strncpy(response.data, "Obstacle DETECTED!\r\n", sizeof(response.data));
        // }
        // taskEXIT_CRITICAL(&myLock);

        //if (bt_receive(&data, portMAX_DELAY) == -1)
        if ( bt_receive(&data, pdMS_TO_TICKS(20)) == -1 )
        {
            // printf("ERROR receiving data from queue.\n");
            motor_stop();
        }
        else
        {
            forward_movement = false;
            switch (data.data[0])
            {
            case 'w':
                forward_movement = true;
                if (!object_detected)
                {
                    motor_forward(0.4f);
                    strncpy(response.data, FORWARD, sizeof(response.data));
                }
                break;
            case 'a':
                strncpy(response.data, LEFT, 64);
                response.data[strlen(LEFT) + 1] = '\0';
                motor_left(0.25f);
                break;
            case 's':
                strncpy(response.data, BACKWARD, 64);
                response.data[strlen(BACKWARD) + 1] = '\0';
                motor_backward(0.4f);
                break;
            case 'd':
                strncpy(response.data, RIGHT, 64);
                response.data[strlen(RIGHT) + 1] = '\0';
                motor_right(0.25f);
                break;
            default:
                // ESP_LOGI("motor", "unknown command");
                strncpy(response.data, UNKNOWN, 64);
                response.data[strlen(UNKNOWN) + 1] = '\0';
                motor_stop();
                break;
            }
            response.length = strlen(response.data);

            if (bt_send(response, pdTICKS_TO_MS(20)) == -1)
            {
                printf("ERROR sending data to client\n");
            }
        }
        // periodically yield control back to OS
        vTaskDelay(1);
    }
}