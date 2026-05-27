#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include "esp_err.h"

esp_err_t motor_driver_init(void);

void motor_enable(void);
void motor_disable(void);
void motor_stop(void);

void motor_forward(float speed);
void motor_backward(float speed);
void motor_left(float speed);
void motor_right(float speed);

/*
 * Direct differential drive control.
 * left_speed/right_speed range:
 * -1.0 = full reverse
 *  0.0 = stop
 *  1.0 = full forward
 */
void motor_drive(float left_speed, float right_speed);

#endif