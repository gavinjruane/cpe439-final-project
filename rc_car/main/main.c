#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "motor_driver.h"
#include "bluetooth.h"

#define GPIO_PIN_2 2

#define FORWARD "Forward\r\n"
#define BACKWARD "Backward\r\n"
#define LEFT "Left\r\n"
#define RIGHT "Right\r\n"
#define UNKNOWN "Unknown\r\n"

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
    BaseType_t result = pdFALSE;

    if ( bt_init("ESP32_DEVICE") == -1 ) {
        printf("ERROR!!\n");
        return;
    }

    motor_driver_init();

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
        vTaskDelay(1);
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
            
            switch ( data.data[0] ) {
                case 'w':
                    strncpy(response.data, FORWARD, 64);
                    response.data[strlen(FORWARD) + 1] = '\0';
                    motor_forward(0.4f);
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
                    ESP_LOGI("motor", "unknown command");
                    strncpy(response.data, UNKNOWN, 64);
                    response.data[strlen(UNKNOWN) + 1] = '\0';
                    motor_stop();
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