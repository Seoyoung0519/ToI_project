// File: PIR_SENSOR.h
#ifndef PIR_SENSOR_H
#define PIR_SENSOR_H

#include "sdkconfig.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/** PIR 센서용 GPIO 핀 번호 */
#define PIR_GPIO                   GPIO_NUM_13
/** HIGH 신호가 이 시간(ms) 이상 지속되면 모션으로 간주 */
#define REQUIRED_HIGH_DURATION_MS  1000
/** 모션이 없다고 보고할 무감지 대기 시간(ms) */
#define NO_MOTION_TIMEOUT_MS       5000

// 내부 상태를 유지하는 변수들
static int     _prev_state       = 0;
static int64_t _high_start_time  = 0;
static int64_t _last_motion_time = 0;
static bool    _motion_triggered = false;
static bool    _no_motion_reported = false;

/**
 * @brief  PIR 센서 GPIO를 초기화하고 내부 상태 변수를 리셋합니다.
 */
static void pir_sensor_init(void)
{
    gpio_set_pull_mode(PIR_GPIO, GPIO_PULLDOWN_ONLY);
    gpio_set_direction(PIR_GPIO, GPIO_MODE_INPUT);
    _prev_state       = gpio_get_level(PIR_GPIO);
    _high_start_time  = _last_motion_time = esp_timer_get_time();
    _motion_triggered = false;
    _no_motion_reported = false;
}

/**
 * @brief  PIR 센서를 읽고
 *         - 일정 시간 HIGH가 유지되면 “Motion Detected” 출력
 *         - LOW로 전환 시 “Motion Ended” 출력
 *         - 무감지 시간 경과 시 “No Motion” 출력
 *         반드시 약 100ms 간격으로 호출하세요.
 */
static void pir_sensor_loop(void)
{
    int     state = gpio_get_level(PIR_GPIO);
    int64_t now   = esp_timer_get_time();  // 마이크로초 단위

    if (state == 1) {
        if (_prev_state == 0) {
            _high_start_time = now;
        }
        if (!_motion_triggered &&
            (now - _high_start_time >= REQUIRED_HIGH_DURATION_MS * 1000)) {
            printf("🟢 Motion Detected (held for %lld ms)\n",
                   (now - _high_start_time) / 1000);
            _motion_triggered   = true;
            _last_motion_time   = now;
            _no_motion_reported = false;
        }
    } else {
        if (_motion_triggered) {
            printf("⚪ Motion Ended\n");
            _motion_triggered = false;
            _last_motion_time = now;
        }
    }

    if (!_no_motion_reported &&
        (now - _last_motion_time >= NO_MOTION_TIMEOUT_MS * 1000)) {
        printf("⚪ No Motion (idle for %lld ms)\n",
               (now - _last_motion_time) / 1000);
        _no_motion_reported = true;
    }

    _prev_state = state;
    vTaskDelay(pdMS_TO_TICKS(100));
}

#endif // PIR_SENSOR_H

