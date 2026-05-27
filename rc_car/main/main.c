#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "bdc_motor.h"
#include "driver/gpio.h"

static const char *TAG = "motor";

#define PWMA 23
#define PWMA_OUTPUT_SELECT (1ULL << PWMA)

#define STBY 22
#define STBY_OUTPUT_SELECT (1ULL << STBY)

#define AIN2 21
#define AIN2_OUTPUT_SELECT (1ULL << AIN2)

#define AIN1 19
#define AIN1_OUTPUT_SELECT (1ULL << AIN1)

#define BDC_MCPWM_TIMER_RESOLUTION_HZ 10000000 // 10MHz
#define BDC_MCPWM_FREQ_HZ             25000    // 25kHz PWM

#define BDC_MCPWM_GPIO_A              23
#define BDC_MCPWM_GPIO_B              15

#define MAX_DUTY (BDC_MCPWM_TIMER_RESOLUTION_HZ / BDC_MCPWM_FREQ_HZ)


// ACTIVE LOW!!!
// Also differential pair? (For scope, we probed positive at AIN2 and negative/"ground" at AIN1, not actual ground)

void app_main(void)
{
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
    gpio_set_level(STBY, 1); // Turn High

    /* DIRECTION */
    gpio_set_level(AIN2, 1); // Turn High
    gpio_set_level(AIN1, 0); // Turn 0



    ESP_LOGI(TAG, "Create motor");

    bdc_motor_config_t motor_config = {
        .pwm_freq_hz = BDC_MCPWM_FREQ_HZ,
        .pwma_gpio_num = BDC_MCPWM_GPIO_A,
        .pwmb_gpio_num = BDC_MCPWM_GPIO_B,
    };

    bdc_motor_mcpwm_config_t mcpwm_config = {
        .group_id = 0,
        .resolution_hz = BDC_MCPWM_TIMER_RESOLUTION_HZ,
    };

    bdc_motor_handle_t motor = NULL;

    ESP_ERROR_CHECK(
        bdc_motor_new_mcpwm_device(
            &motor_config,
            &mcpwm_config,
            &motor
        )
    );

    ESP_LOGI(TAG, "Enable motor");
    ESP_ERROR_CHECK(bdc_motor_enable(motor));

    ESP_LOGI(TAG, "Forward");
    ESP_ERROR_CHECK(bdc_motor_forward(motor));

    while (1) {

        // 25% speed
        ESP_LOGI(TAG, "25%% speed");
        bdc_motor_set_speed(motor, MAX_DUTY * 0.25);
        vTaskDelay(pdMS_TO_TICKS(3000));

        // 50% speed
        ESP_LOGI(TAG, "50%% speed");
        bdc_motor_set_speed(motor, MAX_DUTY * 0.50);
        vTaskDelay(pdMS_TO_TICKS(3000));

        // 75% speed
        ESP_LOGI(TAG, "75%% speed");
        bdc_motor_set_speed(motor, MAX_DUTY * 0.75);
        vTaskDelay(pdMS_TO_TICKS(3000));

        // 100% speed
        ESP_LOGI(TAG, "100%% speed");
        bdc_motor_set_speed(motor, MAX_DUTY - 1);
        vTaskDelay(pdMS_TO_TICKS(3000));

        // Stop
        ESP_LOGI(TAG, "Stop");
        bdc_motor_set_speed(motor, 0);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}




// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/gpio.h"
// #include "driver/ledc.h"

// // #include "motor.h"
// // #include "motor_driver.h"

// #define PWMA 23
// #define PWMA_OUTPUT_SELECT (1ULL << PWMA)

// #define STBY 22
// #define STBY_OUTPUT_SELECT (1ULL << STBY)

// #define AIN2 21
// #define AIN2_OUTPUT_SELECT (1ULL << AIN2)

// #define AIN1 19
// #define AIN1_OUTPUT_SELECT (1ULL << AIN1)


// const int frequency = 60;
// const int pwm_channel = 0;
// const int resolution = 8;

// void app_main(void)
// {
//     // 1. Define the configuration structure
//     gpio_config_t io_conf = {
//         .intr_type = GPIO_INTR_DISABLE, // Disable interrupts
//         .mode = GPIO_MODE_OUTPUT,       // Set as output mode
//         .pin_bit_mask = PWMA_OUTPUT_SELECT | STBY_OUTPUT_SELECT |
//                         AIN2_OUTPUT_SELECT | AIN1_OUTPUT_SELECT,
//         .pull_down_en = 0, // Disable pull-down
//         .pull_up_en = 0    // Disable pull-up
//     };

//     // 2. Apply the configuration
//     gpio_config(&io_conf);

//     // 3. Set the output level
//     /* Must set STBY ON to drive motors */
//     gpio_set_level(STBY, 1); // Turn High

//     /* SET on 1 to go max speed */
//     gpio_set_level(PWMA, 1); // Turn High

//     /* DIRECTION */
//     gpio_set_level(AIN2, 1); // Turn High
//     gpio_set_level(AIN1, 0); // Turn 0

//     while (1) {
//         vTaskDelay(1);
//     }   
// }
