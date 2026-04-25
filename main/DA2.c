#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "bsp_i2c.h"
#include "acs712.h"
#include "hall_sensor.h"
#include "mpu6050.h"
#include "ds3132.h"
#include "protocol/RS485.h"
#include "protocol/USB.h"

static const char *TAG = "DA2";

// ========== Configuration ==========
// Uncomment one of these to select communication method
// #define USE_USB     1
#define USE_RS485   1  // Use 4-pin TTL-RS485 module (VCC/GND/TX/RX)

// Uncomment one of these to select data format
#define SEND_DATA_MODE_TEXT   1  // Send as text (CSV format) - easy to debug
// #define SEND_DATA_MODE_BINARY 1  // Send as binary - compact size

// Data generation interval (200 = 200 microseconds)
#define DATA_GEN_INTERVAL_US 200

// Only log every N packets to avoid blocking too long in the task
#define PACKET_LOG_INTERVAL 10

// ========== Global variables ==========
static uint32_t packet_count = 0;

static void read_sensor_values(int16_t sensor_data[4])
{
    int16_t acc_z = 0;
    uint32_t hall_count = 0;
    float current = 0.0f;

    hall_count = hall_get_count();
    if (mpu6050_read_acc_z(&acc_z) != ESP_OK) {
        acc_z = 0;
    }

    current = acs712_read_current();

    // Real sensor values packed into packet.data[]
    sensor_data[0] = acc_z;
    sensor_data[1] = (int16_t)hall_count;
    sensor_data[2] = (int16_t)(current * 100.0f); // current in 0.01 A units
    sensor_data[3] = 0; // reserved for future sensor data
}

static void rtc_get_string(char *buffer, size_t size)
{
    ds3132_datetime_t datetime;
    if (ds3132_get_time(&datetime) == ESP_OK) {
        ds3132_format_datetime(&datetime, buffer, size);
    } else {
        snprintf(buffer, size, "0000-00-00 00:00:00");
    }
}

static void data_send_task(void *arg)
{
    int16_t sensor_data[4] = {0};
    int64_t last_time = esp_timer_get_time();
    char rtc_text[32] = {0};

#ifdef USE_USB
    usb_packet_t packet;
#endif

#ifdef USE_RS485
    rs485_packet_t packet;
#endif

    ESP_LOGI(TAG, "Data send task started");

    while (1) {
        int64_t current_time = esp_timer_get_time();

        if ((current_time - last_time) >= DATA_GEN_INTERVAL_US) {
            read_sensor_values(sensor_data);

            packet_count++;
            packet.timestamp = (uint32_t)(current_time / 1000);
            packet.sensor_id = 1;
            for (int i = 0; i < 4; i++) {
                packet.data[i] = sensor_data[i];
            }
            packet.checksum = 0;

#ifdef USE_USB
    #ifdef SEND_DATA_MODE_TEXT
            usb_send_data_text(&packet);
    #else
            usb_send_data(&packet);
    #endif
#endif

#ifdef USE_RS485
    #ifdef SEND_DATA_MODE_TEXT
            rtc_get_string(rtc_text, sizeof(rtc_text));
            rs485_send_data_text(&packet, packet_count, rtc_text);
    #else
            rs485_send_data(&packet);
    #endif
#endif

            last_time = current_time;

            if ((packet_count % PACKET_LOG_INTERVAL) == 0) {
                ESP_LOGI(TAG, "Packet %lu sent - time=%s acc_z=%d hall=%d current=%.2fA",
                         packet_count,
                         rtc_text,
                         sensor_data[0],
                         sensor_data[1],
                         sensor_data[2] / 100.0f);
            }

            vTaskDelay(pdMS_TO_TICKS(10));
        }

        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting application");

    esp_log_level_set("*", ESP_LOG_WARN);
    esp_log_level_set(TAG, ESP_LOG_INFO);

    /* 🔥 1. INIT I2C TRƯỚC */
    bsp_i2c_init();
    bsp_i2c_rtc_init();

    /* 🔥 2. INIT SENSOR */
    acs712_init();
    i2c_master_bus_handle_t i2c_bus = bsp_i2c_get_bus_handle();
    esp_err_t err = mpu6050_init(i2c_bus);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "MPU6050 init failed: %s", esp_err_to_name(err));
    }
    hall_init();

    /* 🔥 2.5. INIT RTC */
    i2c_master_bus_handle_t rtc_bus = bsp_i2c_get_rtc_bus_handle();
    err = ds3132_init(rtc_bus);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "DS3132 init failed: %s", esp_err_to_name(err));
    }

    /* 🔥 3. INIT COMMUNICATION */
#ifdef USE_USB
    ESP_LOGI(TAG, "Initializing USB communication");
    usb_init();
#endif

#ifdef USE_RS485
    ESP_LOGI(TAG, "Initializing RS485 communication");
    rs485_init();
#endif

    /* 🔥 4. CREATE SENDING TASK */
    xTaskCreate(
        data_send_task,
        "data_send",
        4096,
        NULL,
        5,
        NULL
    );

    ESP_LOGI(TAG, "Application started - sending data every 200ms");
}
