#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define PIR_GPIO GPIO_NUM_13
#define REQUIRED_HIGH_DURATION_MS 1000     // 감지 지속 조건
#define NO_MOTION_TIMEOUT_MS 5000          // 감지 후 무감 상태 유지 시간

void app_main(void)
{
    gpio_set_pull_mode(PIR_GPIO, GPIO_PULLDOWN_ONLY);
    gpio_set_direction(PIR_GPIO, GPIO_MODE_INPUT);

    int prev_state = 0;
    int state;
    int64_t high_start_time = 0;
    int64_t last_motion_time = 0;
    bool motion_triggered = false;
    bool no_motion_reported = false;

    while (1) {
        state = gpio_get_level(PIR_GPIO);
        int64_t now = esp_timer_get_time();  // 마이크로초 단위

        if (state == 1) {
            if (prev_state == 0) {
                high_start_time = now;
            }

            // 지속된 HIGH 신호가 기준 이상이면 감지
            if (!motion_triggered &&
                now - high_start_time >= REQUIRED_HIGH_DURATION_MS * 1000) {
                printf("🟢 Motion Detected (held for %lld ms)\n", (now - high_start_time) / 1000);
                motion_triggered = true;
                last_motion_time = now;
                no_motion_reported = false;
            }
        } else {
            if (motion_triggered) {
                printf("⚪ Motion Ended\n");
                motion_triggered = false;
                last_motion_time = now;
            }
        }

        // 마지막 감지 이후 일정 시간 동안 아무것도 감지되지 않으면 자동 알림
        if (!no_motion_reported &&
            (now - last_motion_time >= NO_MOTION_TIMEOUT_MS * 1000)) {
            printf("⚪ No Motion (idle for %lld ms)\n", (now - last_motion_time) / 1000);
            no_motion_reported = true;
        }

        prev_state = state;
        vTaskDelay(pdMS_TO_TICKS(100));  // 감지 확인 주기
    }
}
