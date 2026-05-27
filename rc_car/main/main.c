#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "motor.h"
#include "motor_driver.h"

#define PWMA 23
#define PWMA_OUTPUT_SELECT (1ULL << PWMA)

#define STBY 22
#define STBY_OUTPUT_SELECT (1ULL << STBY)

#define AIN2 21
#define AIN2_OUTPUT_SELECT (1ULL << AIN2)

#define AIN1 19
#define AIN1_OUTPUT_SELECT (1ULL << AIN1)

void app_main(void)
{
    // 1. Define the configuration structure
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE, // Disable interrupts
        .mode = GPIO_MODE_OUTPUT,       // Set as output mode
        .pin_bit_mask = PWMA_OUTPUT_SELECT | STBY_OUTPUT_SELECT |
                        AIN2_OUTPUT_SELECT | AIN1_OUTPUT_SELECT,
        .pull_down_en = 0, // Disable pull-down
        .pull_up_en = 0    // Disable pull-up
    };

    // 2. Apply the configuration
    gpio_config(&io_conf);

    // 3. Set the output level
    /* Must set STBY ON to drive motors */
    gpio_set_level(STBY_OUTPUT_SELECT, 1); // Turn High

    /* SET on 1 to go max speed */
    gpio_set_level(PWMA_OUTPUT_SELECT, 1); // Turn High

    /* DIRECTION */
    gpio_set_level(AIN2_OUTPUT_SELECT, 1); // Turn High
    gpio_set_level(AIN1_OUTPUT_SELECT, 0); // Turn 0
}
