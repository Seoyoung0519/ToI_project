// File: Servomotor.c
#include "SERVOMOTOR.h"

void app_main(void) {
    // 1) 모듈 초기화
    servo_init();

    const int step = 30;      // 각도 변화 단위
    const int delay_ms = 500; // 각 위치 지연 시간

    // 2) 0° → 150° → 0° 반복
    while (1) {
        for (int angle = 0; angle <= 150; angle += step) {
            servo_set_angle(angle);
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
        }
        vTaskDelay(pdMS_TO_TICKS(500));

        for (int angle = 150; angle >= 0; angle -= step) {
            servo_set_angle(angle);
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

