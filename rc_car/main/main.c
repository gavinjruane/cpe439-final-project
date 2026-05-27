#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "motor_driver.h"

void app_main(void)
{
    motor_driver_init();

    while (1) {
        motor_forward(0.4f);
        vTaskDelay(pdMS_TO_TICKS(2000));

        motor_left(0.35f);
        vTaskDelay(pdMS_TO_TICKS(1000));

        motor_right(0.35f);
        vTaskDelay(pdMS_TO_TICKS(1000));

        motor_backward(0.4f);
        vTaskDelay(pdMS_TO_TICKS(2000));

        motor_stop();
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

////////////////////////
