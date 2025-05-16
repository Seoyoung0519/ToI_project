// File: PIR_sensor.c
#include "PIR_SENSOR.h"

void app_main(void)
{
    // PIR 센서 모듈 초기화
    pir_sensor_init();

    // 메인 루프: 100ms마다 센서 상태 처리
    while (1) {
        pir_sensor_loop();
    }
}
