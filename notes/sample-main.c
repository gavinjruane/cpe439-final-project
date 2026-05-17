#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_system.h"

void task1 (void *argument);
void task2 (void *argument);

void app_main(void)
{
    TaskHandle_t task1_handle, task2_handle;

    printf("Hello, world! This is a test message!\n");
    printf("Now I will create some test tasks.\n");

    xTaskCreate(
        task1,
        "task1",
        4096,
        NULL,
        2,
        &task1_handle
    );

    xTaskCreate(
        task2,
        "task2",
        4096,
        NULL,
        1,
        &task2_handle
    );
}

void task1 (void *argument) {
    printf("I am Task 1.\n");

    vTaskDelete(NULL);
}

void task2 (void *argument) {
    printf("I am Task 2.\n");

    vTaskDelete(NULL);
}
