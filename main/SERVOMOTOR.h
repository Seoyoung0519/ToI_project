// File: SERVOMOTOR.h
#ifndef SERVOMOTOR_H
#define SERVOMOTOR_H

#include <stdint.h>
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// 서보모터 제어 설정
#define SERVO_GPIO       18
#define LEDC_TIMER       LEDC_TIMER_0
#define LEDC_CHANNEL     LEDC_CHANNEL_0
#define LEDC_MODE        LEDC_LOW_SPEED_MODE
#define LEDC_RESOLUTION  LEDC_TIMER_13_BIT  // 0~8191
#define SERVO_FREQ_HZ    50                 // 50Hz (20ms)

// 각도를 PWM 듀티로 변환 (0°→0.5ms, 180°→2.4ms)
// SG90: 0° → 0.5ms, 180° → 2.08ms 대략
static inline uint32_t angle_to_duty(int angle) {
    const int min_us = 500;
    const int max_us = 2400;
    int us = min_us + (max_us - min_us) * angle / 180;
    // duty = (us / 20000us) * 2^13
    return (us * ((1 << 13) - 1)) / 20000;
}

// 서보모터 초기화: LEDC 타이머 + 채널 설정
static void servo_init(void) {
    ledc_timer_config_t tcfg = {
        .speed_mode      = LEDC_MODE,
        .duty_resolution = LEDC_RESOLUTION,
        .timer_num       = LEDC_TIMER,
        .freq_hz         = SERVO_FREQ_HZ,
        .clk_cfg         = LEDC_AUTO_CLK
    };
    ledc_timer_config(&tcfg);

    ledc_channel_config_t ccfg = {
        .speed_mode = LEDC_MODE,
        .channel    = LEDC_CHANNEL,
        .timer_sel  = LEDC_TIMER,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = SERVO_GPIO,
        .duty       = 0,
        .hpoint     = 0
    };
    ledc_channel_config(&ccfg);
}

// 서보모터 각도 설정
static void servo_set_angle(int angle) {
    uint32_t duty = angle_to_duty(angle);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

#endif // SERVOMOTOR_H
