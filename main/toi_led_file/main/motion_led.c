#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/i2c.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include <math.h>
#include <string.h>

#define TAG "MOTION_LED"

// MPU6500 I2C 설정
#define I2C_MASTER_SCL_IO           9
#define I2C_MASTER_SDA_IO           8
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          100000
#define MPU6500_SENSOR_ADDR         0x68
#define MPU6500_PWR_MGMT_1_REG      0x6B
#define MPU6500_ACCEL_XOUT_H        0x3B

// RGB LED 핀 (공통 캐소드 기준)
#define LEDC_RED_GPIO   3
#define LEDC_GREEN_GPIO 4
#define LEDC_BLUE_GPIO  5

void i2c_master_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

void mpu6500_init() {
    uint8_t cmd[] = {MPU6500_PWR_MGMT_1_REG, 0x00};
    i2c_master_write_to_device(I2C_MASTER_NUM, MPU6500_SENSOR_ADDR, cmd, sizeof(cmd), 1000 / portTICK_PERIOD_MS);

    uint8_t who_am_i = 0;
    i2c_master_write_read_device(I2C_MASTER_NUM, MPU6500_SENSOR_ADDR, (uint8_t[]){0x75}, 1, &who_am_i, 1, 1000 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "WHO_AM_I = 0x%02X", who_am_i);
}

int16_t read_word(uint8_t reg) {
    uint8_t data[2];
    i2c_master_write_read_device(I2C_MASTER_NUM, MPU6500_SENSOR_ADDR, &reg, 1, data, 2, 1000 / portTICK_PERIOD_MS);
    return (int16_t)((data[0] << 8) | data[1]);
}

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

void motion_task(void *arg) {
    float motion_score = 0;
    int count = 0;

    while (1) {
        int16_t ax = read_word(MPU6500_ACCEL_XOUT_H);
        int16_t ay = read_word(MPU6500_ACCEL_XOUT_H + 2);
        int16_t az = read_word(MPU6500_ACCEL_XOUT_H + 4);

        float accel_x = ax / 16384.0f;
        float accel_y = ay / 16384.0f;
        float accel_z = az / 16384.0f;

        float acc_mag = sqrtf(accel_x * accel_x + accel_y * accel_y + accel_z * accel_z);
        float delta = fabsf(acc_mag - 1.0f);  // 중력 제거

        if (delta > 0.2f) {
            motion_score += delta;
        }

        // 🟢 실시간 LED 색상 출력
        if (motion_score < 550) {
            set_led_color(0, 255, 0);  // 초록
        } else {
            set_led_color(255, 255, 255);  // 빨강
        }
        
        // 🔴 30초마다 평가 후 리셋
        if (count >= 300) {
            char level[16];
            if (motion_score < 300) strcpy(level, "low");
            else if (motion_score < 550) strcpy(level, "medium");
            else strcpy(level, "high");

            char msg[64];
            snprintf(msg, sizeof(msg), "Activity:%s Score:%.1f", level, motion_score);
            ESP_LOGI(TAG, "[30초 결과] %s", msg);

            motion_score = 0;
            count = 0;
        }

        count++;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void app_main() {
    i2c_master_init();
    mpu6500_init();
    rgb_led_init();
    xTaskCreate(motion_task, "motion_task", 4096, NULL, 5, NULL);
}