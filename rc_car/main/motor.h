#ifndef MOTOR_H
#define MOTOR_H

int motor_init(void);
void move_forward(int speed);
void move_backward(int speed);
void move_left(int speed);
void move_right(int speed);
void motor_task(void *arg);
void input_task(void *arg);

#endif