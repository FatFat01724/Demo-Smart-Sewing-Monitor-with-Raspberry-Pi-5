#pragma once
#include "esp_err.h"
#include <stdint.h>
#include "driver/i2c_master.h"

esp_err_t mpu6050_init(i2c_master_bus_handle_t bus_handle);
esp_err_t mpu6050_read_acc_z(int16_t *acc_z);
