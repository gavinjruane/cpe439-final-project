#ifndef ULTRASONIC_H
#define ULTRASONIC_H 


#include <stdint.h>
#include "esp_err.h"
#include "driver/gpio.h"

typedef struct {
    gpio_num_t trig_pin;
    gpio_num_t echo_pin;
    uint32_t timeout_us;
} hcsr04_t;

esp_err_t hcsr04_init(hcsr04_t *sensor, gpio_num_t trig_pin, gpio_num_t echo_pin);

esp_err_t hcsr04_read_cm(hcsr04_t *sensor, float *distance_cm);

esp_err_t hcsr04_read_cm_filtered(hcsr04_t *sensor, float *distance_cm);
#endif



