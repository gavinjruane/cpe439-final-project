#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

#include "bluetooth.h"

#define GPIO_PIN_2 2

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
            struct bt_data send_this = { .data = "Received data.\r\n", .length = strlen("Received data.\n") };
            if ( bt_send(send_this, portMAX_DELAY) == -1 ) {
                printf("ERROR sending data to host\n");
            }
        }

        // periodically yield control back to OS
        vTaskDelay(1);
    }
}