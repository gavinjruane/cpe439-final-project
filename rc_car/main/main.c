#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LED_PIN 3


void app_main(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_PIN), // Select the pin(s)
        .mode = GPIO_MODE_OUTPUT,          // Set as output
        .pull_up_en = GPIO_PULLUP_DISABLE, // Disable pull-up
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // Disable pull-down
        .intr_type = GPIO_INTR_DISABLE      // Disable interrupts
    };

    // 2. Apply the configuration
    gpio_config(&io_conf);

    // 3. Set the level
    gpio_set_level(LED_PIN, 1); // Turn ON (High)

    while (1) {
        gpio_set_level(LED_PIN, 1); // Turn ON (High)
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
        gpio_set_level(LED_PIN, 0); // Turn OFF (Low)
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
    }
}


