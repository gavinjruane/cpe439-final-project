#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define MOTOR1_STEP 
#define MOTOR1_DIR 

#define MOTOR2_STEP
#define MOTOR2_DIR

#define MOTOR3_STEP
#define MOTOR3_DIR

#define MOTOR4_STEP
#define MOTOR4_DIR




int motor_init(void) {
    // Initialize motor control pins here
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << MOTOR1_STEP) | (1ULL << MOTOR1_DIR) | (1ULL << MOTOR2_STEP) |
                        (1ULL << MOTOR2_DIR) | (1ULL << MOTOR3_STEP) | (1ULL << MOTOR3_DIR) | 
                        (1ULL << MOTOR4_STEP) | (1ULL << MOTOR4_DIR),
        .mode = GPIO_MODE_OUTPUT,          // Set as output
        .pull_up_en = GPIO_PULLUP_DISABLE, // Disable pull-up
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // Disable pull-down
        .intr_type = GPIO_INTR_DISABLE      // Disable interrupts
    };

    //apply the configuration
    gpio_config(&io_conf);

    
    return 0; // Return 0 on success
}