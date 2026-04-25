#include <stdio.h>
#include "data_collector.h"

#include "hall_sensor.h"
#include "acs712.h"
#include "mpu6050.h"

#include "edge_task.h"
#include "machine_info.h"

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void data_collector_task(void *arg)
{
    int16_t acc_z = 0;

    edge_task_init();

    printf("DATA COLLECTOR STARTED\n");

    while (1)
    {
        uint32_t now = esp_timer_get_time() / 1000;

        int hall = hall_get_count();      // 0 hoặc 1
        float current = acs712_read_current();

        if (mpu6050_read_acc_z(&acc_z) != ESP_OK)
            acc_z = 0;

        edge_task_update(hall, current, acc_z, now);

        machine_info_add_stitch(edge_get_stitch_delta());
        machine_info_set_rpm(edge_get_rpm());
        machine_info_set_running(edge_is_running());

        /* 🔥 LOG KIỂM TRA DỮ LIỆU */
        printf("hall=%d  acc_z=%d  current=%.2f  rpm=%lu  run=%d\n",
       hall,
       acc_z,
       current,
       edge_get_rpm(),        // uint32_t
       edge_is_running());


        /* ⚠️ DELAY ĐỦ LỚN ĐỂ UART IN */
        vTaskDelay(pdMS_TO_TICKS(200));   // 200 ms
    }
}
