#include <stdio.h>
#include <stdint.h>

#include "ultrasonic.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* DEFINES */
#define HSR04_TIMEOUT_US 30000

static const char *TAG = "HCSR04";

esp_err_t hcsr04_init(hcsr04_t *sensor, gpio_num_t trig_pin, gpio_num_t echo_pin)
{
    sensor->trig_pin = trig_pin;
    sensor->echo_pin = echo_pin;
    sensor->timeout_us = HSR04_TIMEOUT_US;

    gpio_reset_pin(sensor->trig_pin);
    gpio_reset_pin(sensor->echo_pin);

    gpio_set_direction(sensor->trig_pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(sensor->echo_pin, GPIO_MODE_INPUT);

    gpio_set_level(sensor->trig_pin, 0);
    ESP_LOGI(TAG, "Intializaed HC-SR04: TRIG = %d, ECHO = %d",
             sensor->trig_pin, sensor->echo_pin);

    return ESP_OK;
}

esp_err_t hcsr04_read_cm(hcsr04_t *sensor, float *distance_cm)
{
    if (sensor == NULL || distance_cm == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    int64_t timeout_start = 0;
    int64_t start_time = 0;
    int64_t end_time = 0;

    /*
     * Step 1:
     * Send a clean 10 us trigger pulse.
     */
    gpio_set_level(sensor->trig_pin, 0);
    esp_rom_delay_us(2);

    gpio_set_level(sensor->trig_pin, 1);
    esp_rom_delay_us(10);

    gpio_set_level(sensor->trig_pin, 0);

    /*
     * Step 2:
     * Wait for ECHO to go HIGH.
     */
    timeout_start = esp_timer_get_time();

    while (gpio_get_level(sensor->echo_pin) == 0)
    {
        if ((esp_timer_get_time() - timeout_start) > sensor->timeout_us)
        {
            return ESP_ERR_TIMEOUT;
        }
    }

    start_time = esp_timer_get_time();

    /*
     * Step 3:
     * Wait for ECHO to go LOW.
     */
    timeout_start = esp_timer_get_time();

    while (gpio_get_level(sensor->echo_pin) == 1)
    {
        if ((esp_timer_get_time() - timeout_start) > sensor->timeout_us)
        {
            return ESP_ERR_TIMEOUT;
        }
    }

    end_time = esp_timer_get_time();

    /*
     * Step 4:
     * Convert pulse width to centimeters.
     *
     * HC-SR04 common formula:
     * distance_cm = pulse_width_us / 58.0
     */
    int64_t pulse_width_us = end_time - start_time;

    *distance_cm = pulse_width_us / 58.0f;

    return ESP_OK;
}

esp_err_t hcsr04_read_cm_filtered(hcsr04_t *sensor, float *distance_cm)
{
    float samples[5];
    int valid_count = 0;

    for (int i = 0; i < 5; i++)
    {
        float d = 0.0f;

        if (hcsr04_read_cm(sensor, &d) == ESP_OK)
        {
            samples[valid_count] = d;
            valid_count++;
        }

        vTaskDelay(pdMS_TO_TICKS(60));
    }

    if (valid_count == 0)
    {
        return ESP_ERR_TIMEOUT;
    }

    float sum = 0.0f;

    for (int i = 0; i < valid_count; i++)
    {
        sum += samples[i];
    }

    *distance_cm = sum / valid_count;

    return ESP_OK;
}
