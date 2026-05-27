#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"

#define MOTORBR_STEP 
#define MOTORBR_DIR 

#define MOTORFR_STEP
#define MOTORFR_DIR

#define MOTORBL_STEP
#define MOTORBL_DIR

#define MOTORFL_STEP
#define MOTORFL_DIR

//minimum step delay specified by the datasheet is 1 us so we are using 2 to be safe
#define MIN_STEP_DELAY 2 

#define FORWARD 0
#define BACKWARD 1




int motor_init(void) {
    // Initialize motor control pins here
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << MOTORFL_STEP) | (1ULL << MOTORFL_DIR) | (1ULL << MOTORFR_STEP) | (1ULL << MOTORFR_DIR) |
                        (1ULL << MOTORBL_STEP) | (1ULL << MOTORBL_DIR) | (1ULL << MOTORBR_STEP) | (1ULL << MOTORBR_DIR),
        .mode = GPIO_MODE_OUTPUT,          // Set as output
        .pull_up_en = GPIO_PULLUP_DISABLE, // Disable pull-up
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // Disable pull-down
        .intr_type = GPIO_INTR_DISABLE      // Disable interrupts
    };

    //apply the configuration
    gpio_config(&io_conf);

    
    return 0; // Return 0 on success
}


void move_forward(int speed) {

    //this function is made to be used in the following manner: 
    // while (forward input is active) {
    //     move_forward(desired_speed)}

    //speed is in units steps per second

    //all motors moving forward
    gpio_set_level(MOTORBL_DIR, FORWARD);
    gpio_set_level(MOTORFL_DIR, FORWARD);
    gpio_set_level(MOTORBR_DIR, FORWARD);
    gpio_set_level(MOTORFR_DIR, FORWARD);
    

    step_all_motors(speed);
    

}

void move_backward(int speed) {


    //speed is in units steps per second

    //all motors moving backward
    gpio_set_level(MOTORBL_DIR, BACKWARD);
    gpio_set_level(MOTORFL_DIR, BACKWARD);
    gpio_set_level(MOTORBR_DIR, BACKWARD);
    gpio_set_level(MOTORFR_DIR, BACKWARD);

    step_all_motors(speed);
    

}

void move_left(int speed) {
    //right motors forward, left motors backward
    gpio_set_level(MOTORBL_DIR, BACKWARD);
    gpio_set_level(MOTORFL_DIR, BACKWARD);
    gpio_set_level(MOTORBR_DIR, FORWARD);
    gpio_set_level(MOTORFR_DIR, FORWARD);

    step_all_motors(speed);
}

void move_right(int speed) {
    //left motors forward, right motors backward
    gpio_set_level(MOTORBL_DIR, FORWARD);
    gpio_set_level(MOTORFL_DIR, FORWARD);
    gpio_set_level(MOTORBR_DIR, BACKWARD);
    gpio_set_level(MOTORFR_DIR, BACKWARD);

    step_all_motors(speed);
}

void step_all_motors(int speed)
{
    if (speed <= 0) {
        return;
    }

    int period_us = 1000000 / speed;
    int low_delay_us = period_us - MIN_STEP_DELAY;

    if (low_delay_us < MIN_STEP_DELAY) {
        low_delay_us = MIN_STEP_DELAY;
    }

    //create rising edge to step all motors
    gpio_set_level(MOTORBL_STEP, 1);
    gpio_set_level(MOTORFL_STEP, 1);
    gpio_set_level(MOTORBR_STEP, 1);
    gpio_set_level(MOTORFR_STEP, 1);

    esp_rom_delay_us(MIN_STEP_DELAY);

    gpio_set_level(MOTORBL_STEP, 0);
    gpio_set_level(MOTORFL_STEP, 0);
    gpio_set_level(MOTORBR_STEP, 0);
    gpio_set_level(MOTORFR_STEP, 0);

    esp_rom_delay_us(low_delay_us);
}


// task structure for the motor control task, not everything is defined yet
void motor_task(void *arg)
{
    while (1) {
        switch (motor_cmd) {
            case CMD_FORWARD:
                move_forward(motor_speed);
                break;

            case CMD_BACKWARD:
                move_backward(motor_speed);
                break;

            case CMD_LEFT:
                turn_left(motor_speed);
                break;

            case CMD_RIGHT:
                turn_right(motor_speed);
                break;

            case CMD_STOP:
            default:
                vTaskDelay(pdMS_TO_TICKS(10));
                break;
        }
    }
}

//structure for the motor input handeling task, functions are not yet defined
void input_task(void *arg)
{
    while (1) {
        if (forward_input_active()) {
            motor_cmd = CMD_FORWARD;
        } else if (backward_input_active()) {
            motor_cmd = CMD_BACKWARD;
        } else if (left_input_active()) {
            motor_cmd = CMD_LEFT;
        } else if (right_input_active()) {
            motor_cmd = CMD_RIGHT;
        } else {
            motor_cmd = CMD_STOP;
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}