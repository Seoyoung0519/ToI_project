#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "dummy_motion_data.h"
#include "SERVOMOTOR.h"

#define TAG "SERVO_DEMO"
#define ROTATE_THRESHOLD 600.0f

void app_main(void) { 
    servo_init();

    int line_idx = 0;
    const int total_lines = sizeof(dummy_data) / sizeof(dummy_data[0]);

    int current_angle = 0;  // 현재 각도
    int direction = 1;      // 1이면 증가, -1이면 감소

    while (1) {
        if (line_idx >= total_lines) {
            ESP_LOGW(TAG, "모든 더미 데이터를 다 읽었습니다. 프로그램 종료 혹은 반복하세요.");
            break;
        }

        const char* line = dummy_data[line_idx++];
        ESP_LOGI(TAG, "[DEBUG] Read line: %s", line);

        float score = 0.0f;
        if (sscanf(line, "Activity:%*[^ ] Score:%f", &score) == 1) {
            ESP_LOGI(TAG, "[DEBUG] Parsed score: %.1f", score);

            if (score >= ROTATE_THRESHOLD) {
                current_angle += direction * 30;

                // 방향 전환 조건
                if (current_angle >= 150) {
                    current_angle = 150;
                    direction = -1;
                } else if (current_angle <= 0) {
                    current_angle = 0;
                    direction = 1;
                }

                ESP_LOGI(TAG, "[ACTION] Score %.1f is high → rotating servo to %d degrees.", score, current_angle);
                servo_set_angle(current_angle);
            } else {
                ESP_LOGI(TAG, "[ACTION] Score %.1f is low → servo angle 유지 (%d도)", score, current_angle);
                // 유지하므로 servo_set_angle 생략 가능
            }
        } else {
            ESP_LOGE(TAG, "Failed to parse line: %s", line);
        }

        vTaskDelay(pdMS_TO_TICKS(30000));  // 30초마다 처리
    }
}
