#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include "dummy_motion_data.h"  // 문자열 배열로 되어 있어야 함

#define TAG "LED_TEST"
#define LEDC_RED_GPIO   15
#define LEDC_GREEN_GPIO 2
#define LEDC_BLUE_GPIO  4

void rgb_led_init() {
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_8_BIT,
        .freq_hz          = 5000,
        .clk_cfg          = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&ledc_timer);

    int channels[3] = {LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2};
    int gpios[3] = {LEDC_RED_GPIO, LEDC_GREEN_GPIO, LEDC_BLUE_GPIO};

    for (int i = 0; i < 3; i++) {
        ledc_channel_config_t ledc_channel = {
            .speed_mode     = LEDC_LOW_SPEED_MODE,
            .channel        = channels[i],
            .timer_sel      = LEDC_TIMER_0,
            .intr_type      = LEDC_INTR_DISABLE,
            .gpio_num       = gpios[i],
            .duty           = 0,
            .hpoint         = 0,
        };
        ledc_channel_config(&ledc_channel);
    }
}

void set_led_color(uint8_t r, uint8_t g, uint8_t b) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, r);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, g);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, b);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
}

void update_led_by_score(float score) {
    if (score < 100) {
        set_led_color(0, 255, 0);   // 초록
    } else if (score < 200) {
        set_led_color(0, 0, 255);   // 파랑
    } else {
        set_led_color(255, 0, 0);   // 빨강
    }
}

void test_led_task(void *arg) {
    int idx = 0;
    int total = sizeof(dummy_data) / sizeof(dummy_data[0]);
    float accumulated_score = 0.0f;
    int count = 0;

    while (1) {
        if (idx >= total) {
            idx = 0; // dummy_data 반복
        }

        float score = 0.0f;
        const char* line = dummy_data[idx++];
        ESP_LOGI(TAG, "[DEBUG] Read line: %s", line);

        if (sscanf(line, "Activity:%*[^ ] Score:%f", &score) == 1) {
            accumulated_score += score;
            count++;

            if (count >= 10) {  // 0.5초 x 10 = 5초마다 실행
                // 누적값 출력
                const char* level;
                if (accumulated_score < 100) level = "low";
                else if (accumulated_score < 200) level = "medium";
                else level = "high";

                ESP_LOGI(TAG, "🔴 5초 누적 운동량: %.1f (%s)", accumulated_score, level);

                // LED 색상 한 번만 변경
                update_led_by_score(accumulated_score);

                // 초기화
                accumulated_score = 0.0f;
                count = 0;
            }
        } else {
            ESP_LOGE(TAG, "파싱 실패: %s", line);
        }

        vTaskDelay(pdMS_TO_TICKS(500));  // 0.5초 간격
    }
}

void app_main() {
    rgb_led_init();
    xTaskCreate(test_led_task, "test_led_task", 4096, NULL, 5, NULL);
}
