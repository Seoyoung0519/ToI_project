// File: motion_ble_servo.c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/ledc.h"
#include "esp_log.h"
#include "SERVOMOTOR.h"

#define TAG "BLE_SERVO"

#define ROTATE_THRESHOLD 600.0f  // 회전 기준 점수
#define MAX_TOTAL 6              // 하루 최대 수신 횟수
#define MAX_ANGLE 150            // 최대 각도 제한

// 내부 상태 변수
static int total = 0;
static int rotation = 0;
static int current_angle = 0;

/// 상태 초기화 함수
void reset_servo_state() {
    total = 0;
    rotation = 0;
    current_angle = 0;
}

/// 150 → 120 → 150 → 0 회전 시퀀스
void handle_servo_reaction() {
    if (rotation < 5) {
        ESP_LOGI(TAG, "rotation < 5 → 0도 복귀");
        servo_set_angle(0);
    } else {
        ESP_LOGI(TAG, "rotation ≥ 5 → 반응 시퀀스 수행");

        servo_set_angle(120);
        vTaskDelay(pdMS_TO_TICKS(1000));

        servo_set_angle(150);
        vTaskDelay(pdMS_TO_TICKS(1000));

        servo_set_angle(0);
    }
}

/// BLE 수신 문자열을 분석하고 서보 회전 처리
void handle_score_string(const char* line) {
    float score = 0.0f;

    if (sscanf(line, "Activity:%*[^ ] Score:%f", &score) == 1) {
        ESP_LOGI(TAG, "[파싱 성공] Score: %.1f", score);

        total++;

        if (score >= ROTATE_THRESHOLD) {
            rotation++;
            current_angle += 30;
            if (current_angle > MAX_ANGLE) current_angle = MAX_ANGLE;

            ESP_LOGI(TAG, "Score ≥ 600 → %d도 회전", current_angle);
            servo_set_angle(current_angle);
        } else {
            ESP_LOGI(TAG, "Score < 600 → 회전 유지 (%d도)", current_angle);
        }

        if (total == MAX_TOTAL || rotation == 5) {
            handle_servo_reaction();
            reset_servo_state();
        }

    } else {
        ESP_LOGW(TAG, "[파싱 실패] 올바른 문자열 아님: %s", line);
    }
}
