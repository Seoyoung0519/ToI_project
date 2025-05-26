#include "PIRSENSOR.h"
#include "SERVOMOTOR.h"

void app_main(void) {
    pir_sensor_init();
    servo_init();

    const int step = 30;
    int current_angle = 0;
    int direction = 1;

    while (1) {
        if (pir_motion_detected()) {
            printf("🟢 Motion Detected\n");

            current_angle += step * direction;
            if (current_angle >= 150) {
                current_angle = 150;
                direction = -1;
            } else if (current_angle <= 0) {
                current_angle = 0;
                direction = 1;
            }

            servo_set_angle(current_angle);
        }

        if (pir_motion_ended()) {
            printf("⚪ Motion Ended\n");
        }

        if (pir_no_motion()) {
            printf("⚪ No Motion\n");
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
