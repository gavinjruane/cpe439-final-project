#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

#define GPIO_PIN_2 2

void app_main(void) {
    gpio_reset_pin(GPIO_PIN_2);

    gpio_set_direction(GPIO_PIN_2, GPIO_MODE_OUTPUT);

    int led_state = 0;
    while (1) {
        gpio_set_level(GPIO_PIN_2, led_state);

        printf("LED is %s\n", led_state ? "ON" : "OFF");

        led_state = !led_state;

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}