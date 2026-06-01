#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "bluetooth.h"
#include "ultrasonic.h"

#define GPIO_PIN_2 2

#define FORWARD "Forward\r\n"
#define BACKWARD "Backward\r\n"
#define LEFT "Left\r\n"
#define RIGHT "Right\r\n"
#define UNKNOWN "Unknown\r\n"

#define SAFE_DIST_CM 25.0f
static hcsr04_t front_sensor; 

void bt_sample_task (void *argument);

TaskHandle_t bt_sample_task_handle;

void app_main(void) {
    // gpio_reset_pin(GPIO_PIN_2);

    // gpio_set_direction(GPIO_PIN_2, GPIO_MODE_OUTPUT);

    // int led_state = 0;
    // while (1) {
    //     gpio_set_level(GPIO_PIN_2, led_state);

    //     printf("LED is %s\n", led_state ? "ON" : "OFF");

    //     led_state = !led_state;

    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }
    if (hcsr04_init(&front_sensor, GPIO_NUM_13, GPIO_NUM_34) != ESP_OK)
    {
        printf("Failed to initialize HC-SR04\n");
        return;
    }

    BaseType_t result = pdFALSE;

    if ( bt_init("ESP32_DEVICE") == -1 ) {
        printf("ERROR!!\n");
        return;
    }

    result = xTaskCreate(
        bt_sample_task,
        "BT_Sample_Task",
        8192,
        NULL,
        1,
        &bt_sample_task_handle
    );
    if ( result == pdFALSE ) {
        printf("Could not create bt_sample_task.\n");
        return;
    }

    while (1) {

        float distance_cm = 0.0f;
        esp_err_t err = hcsr04_read_cm_filtered(&front_sensor, &distance_cm);

        if (err == ESP_OK) {
            printf("Distance: %.2f cm\n", distance_cm);
        } else if (err == ESP_ERR_TIMEOUT) {
            printf("HC-SR04 timeout\n");
        } else {
            printf("HC-SR04 read error\n");
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void bt_sample_task (void *argument) {
    struct bt_data data = { 0 };

    while (1) {
        if ( bt_receive(&data, portMAX_DELAY) == -1 ) {
            printf("ERROR receiving data from queue.\n");
        } else {
            printf("Data: %s, Length: %d\n", data.data, data.length);
            struct bt_data response = { 0 };
            
            float dist_cm = 0.0f;
            esp_err_t dist_err = hcsr04_read_cm_filtered(&front_sensor, &dist_cm);

            if (dist_err == ESP_OK)
            {
                printf("Distance: %.2f cm\n", dist_cm);
            }
            else
            {
                printf("Distance read failed\n");
            }

            switch ( data.data[0] ) {
                case 'w':

                if (dist_err == ESP_OK && dist_cm < SAFE_DIST_CM)
                {
                    motor_stop();
                    snprintf(response.data, sizeof(response.data), 
                        "Obstacle %.2f cm: STOP\r\n", dist_cm);
                }
                else
                {
                    motor_forward();
                    strncpy(response.data, FORWARD, sizeof(response.data));
                }
                    strncpy(response.data, FORWARD, 64);
                    response.data[strlen(FORWARD) + 1] = '\0';
                    break;
                case 'a':
                    strncpy(response.data, LEFT, 64);
                    response.data[strlen(LEFT) + 1] = '\0';
                    break;
                case 's':
                    strncpy(response.data, BACKWARD, 64);
                    response.data[strlen(BACKWARD) + 1] = '\0';
                    break;
                case 'd':
                    strncpy(response.data, RIGHT, 64);
                    response.data[strlen(RIGHT) + 1] = '\0';
                    break;
                default:
                    ESP_LOGI("motor", "unknown command");
                    strncpy(response.data, UNKNOWN, 64);
                    response.data[strlen(UNKNOWN) + 1] = '\0';
                    break;
            }

            response.length = strlen(response.data);

            if ( bt_send(response, portMAX_DELAY) == -1 ) { 
                printf("ERROR sending data to client\n");
            }
        }

        // periodically yield control back to OS
        vTaskDelay(1);
    }
}