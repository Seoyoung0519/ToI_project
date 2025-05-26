// File: PIR_SENSOR.h
#ifndef PIRSENSOR_H
#define PIRSENSOR_H

#include "sdkconfig.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define PIR_GPIO                   GPIO_NUM_13
#define REQUIRED_HIGH_DURATION_MS  700
#define NO_MOTION_TIMEOUT_MS       5000

static int     _prev_state       = 0;
static int64_t _high_start_time  = 0;
static int64_t _last_motion_time = 0;
static bool    _motion_triggered = false;
static bool    _no_motion_reported = false;

static void pir_sensor_init(void) {
    gpio_set_pull_mode(PIR_GPIO, GPIO_PULLDOWN_ONLY);
    gpio_set_direction(PIR_GPIO, GPIO_MODE_INPUT);
    _prev_state = gpio_get_level(PIR_GPIO);
    _high_start_time = _last_motion_time = esp_timer_get_time();
    _motion_triggered = false;
    _no_motion_reported = false;
}

static bool pir_motion_detected(void) {
    int state = gpio_get_level(PIR_GPIO);
    int64_t now = esp_timer_get_time();

    if (state == 1) {
        if (_prev_state == 0) {
            _high_start_time = now;
        }

        if (!_motion_triggered &&
            (now - _high_start_time >= REQUIRED_HIGH_DURATION_MS * 1000)) {
            _motion_triggered = true;
            _last_motion_time = now;
            _no_motion_reported = false;
            _prev_state = state;
            return true;
        }
    }
    _prev_state = state;
    return false;
}

static bool pir_motion_ended(void) {
    int state = gpio_get_level(PIR_GPIO);
    int64_t now = esp_timer_get_time();

    if (_motion_triggered && state == 0) {
        _motion_triggered = false;
        _last_motion_time = now;
        return true;
    }
    return false;
}

static bool pir_no_motion(void) {
    int64_t now = esp_timer_get_time();

    if (!_no_motion_reported &&
        (now - _last_motion_time >= NO_MOTION_TIMEOUT_MS * 1000)) {
        _no_motion_reported = true;
        return true;
    }
    return false;
}

#endif // PIR_SENSOR_H
