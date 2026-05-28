#include "motor_driver.h"

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "motor_driver";

/* Change these pins to match your wiring */
#define STBY_1 22
#define STBY_2 27

/* Motor Driver A-Right */
#define FL_PWM 23
#define FL_IN1 19
#define FL_IN2 21

/* Motor Driver B-Right */
#define FR_PWM 18
#define FR_IN1 5
#define FR_IN2 4

/* Motor Driver A-Left */
#define BL_PWM 34
#define BL_IN1 35
#define BL_IN2 32

/* Motor Driver B-Left */
#define BR_PWM 33
#define BR_IN1 25
#define BR_IN2 26

#define PWM_FREQ_HZ 25000
#define PWM_RESOLUTION LEDC_TIMER_10_BIT
#define PWM_MAX_DUTY 1023

typedef struct
{
    gpio_num_t pwm_pin;
    gpio_num_t in1_pin;
    gpio_num_t in2_pin;
    ledc_channel_t channel;
} motor_t;

static motor_t front_left = {
    .pwm_pin = FL_PWM,
    .in1_pin = FL_IN1,
    .in2_pin = FL_IN2,
    .channel = LEDC_CHANNEL_0};

static motor_t front_right = {
    .pwm_pin = FR_PWM,
    .in1_pin = FR_IN1,
    .in2_pin = FR_IN2,
    .channel = LEDC_CHANNEL_1};

static motor_t back_left = {
    .pwm_pin = BL_PWM,
    .in1_pin = BL_IN1,
    .in2_pin = BL_IN2,
    .channel = LEDC_CHANNEL_2};

static motor_t back_right = {
    .pwm_pin = BR_PWM,
    .in1_pin = BR_IN1,
    .in2_pin = BR_IN2,
    .channel = LEDC_CHANNEL_3};

static float clamp_speed(float speed)
{
    if (speed > 1.0f)
        return 1.0f;
    if (speed < -1.0f)
        return -1.0f;
    return speed;
}

static uint32_t speed_to_duty(float speed)
{
    if (speed < 0.0f)
    {
        speed = -speed;
    }

    if (speed > 1.0f)
    {
        speed = 1.0f;
    }

    return (uint32_t)(speed * PWM_MAX_DUTY);
}

static esp_err_t motor_init_single(motor_t *motor)
{
    gpio_config_t dir_config = {
        .pin_bit_mask = (1ULL << motor->in1_pin) | (1ULL << motor->in2_pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};

    ESP_ERROR_CHECK(gpio_config(&dir_config));

    ledc_channel_config_t pwm_config = {
        .gpio_num = motor->pwm_pin,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = motor->channel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0};

    return ledc_channel_config(&pwm_config);
}

static void motor_set(motor_t *motor, float speed)
{
    speed = clamp_speed(speed);

    if (speed > 0.0f)
    {
        gpio_set_level(motor->in1_pin, 1);
        gpio_set_level(motor->in2_pin, 0);
    }
    else if (speed < 0.0f)
    {
        gpio_set_level(motor->in1_pin, 0);
        gpio_set_level(motor->in2_pin, 1);
    }
    else
    {
        gpio_set_level(motor->in1_pin, 0);
        gpio_set_level(motor->in2_pin, 0);
    }

    ledc_set_duty(LEDC_LOW_SPEED_MODE, motor->channel, speed_to_duty(speed));
    ledc_update_duty(LEDC_LOW_SPEED_MODE, motor->channel);
}

esp_err_t motor_driver_init(void)
{
    ESP_LOGI(TAG, "Initializing TB6612FNG motor driver");

    gpio_config_t stby_config = {
        .pin_bit_mask = (1ULL << STBY_1) | (1ULL << STBY_2),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};

    ESP_ERROR_CHECK(gpio_config(&stby_config));

    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = PWM_RESOLUTION,
        .freq_hz = PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK};

    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    ESP_ERROR_CHECK(motor_init_single(&front_left));
    ESP_ERROR_CHECK(motor_init_single(&front_right));
    ESP_ERROR_CHECK(motor_init_single(&back_left));
    ESP_ERROR_CHECK(motor_init_single(&back_right));

    motor_enable();
    motor_stop();

    return ESP_OK;
}

void motor_enable(void)
{
    gpio_set_level(STBY_1, 1);
    gpio_set_level(STBY_2, 1);
}

void motor_disable(void)
{
    motor_stop();
    gpio_set_level(STBY_1, 0);
    gpio_set_level(STBY_2, 0);
}

void motor_stop(void)
{
    motor_set(&front_left, 0.0f);
    motor_set(&front_right, 0.0f);
    motor_set(&back_left, 0.0f);
    motor_set(&back_right, 0.0f);
}

void motor_drive(float left_speed, float right_speed)
{
    motor_set(&front_left, left_speed);
    motor_set(&back_left, left_speed);

    motor_set(&front_right, right_speed);
    motor_set(&back_right, right_speed);
}

void motor_forward(float speed)
{
    motor_drive(speed, speed);
}

void motor_backward(float speed)
{
    motor_drive(-speed, -speed);
}

void motor_left(float speed)
{
    motor_drive(-speed, speed);
}

void motor_right(float speed)
{
    motor_drive(speed, -speed);
}