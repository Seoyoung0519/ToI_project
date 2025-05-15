#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include <stdio.h>

#define SERVO_GPIO       18
#define LEDC_TIMER       LEDC_TIMER_0
#define LEDC_CHANNEL     LEDC_CHANNEL_0
#define LEDC_MODE        LEDC_LOW_SPEED_MODE
#define LEDC_RESOLUTION  LEDC_TIMER_13_BIT  // 0~8191
#define SERVO_FREQ_HZ    50                 // 20ms 주기

// SG90: 0° → 0.5ms, 180° → 2.08ms 대략
static inline uint32_t angle_to_duty(int angle) {
    const int min_us = 500;
    const int max_us = 2400;
    int us = min_us + (max_us - min_us) * angle / 180;
    // duty = (us / 20000us) * 2^13
    return (us * ((1 << 13) - 1)) / 20000;
}

void set_servo_angle(int angle) {
    uint32_t duty = angle_to_duty(angle);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

void app_main(void)
{
    // 1) LEDC 타이머 설정 (50Hz)
    ledc_timer_config_t timer_cfg = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_RESOLUTION,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = SERVO_FREQ_HZ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_cfg);

    // 2) LEDC 채널 설정 (GPIO18에 PWM 출력 연결)
    ledc_channel_config_t channel_cfg = {
        .speed_mode   = LEDC_MODE,
        .channel      = LEDC_CHANNEL,
        .timer_sel    = LEDC_TIMER,
        .intr_type    = LEDC_INTR_DISABLE,
        .gpio_num     = SERVO_GPIO,
        .duty         = 0,
        .hpoint       = 0
    };
    ledc_channel_config(&channel_cfg);

    const int step = 30;
    const int delay_ms = 500;

    while (1) {
        // 0° → 150°
        for (int angle = 0; angle <= 150; angle += step) {
            set_servo_angle(angle);
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
        }
        vTaskDelay(pdMS_TO_TICKS(500));

        // 150° → 0°
        for (int angle = 150; angle >= 0; angle -= step) {
            set_servo_angle(angle);
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}



