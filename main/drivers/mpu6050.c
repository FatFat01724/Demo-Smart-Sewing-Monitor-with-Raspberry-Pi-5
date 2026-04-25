#include "mpu6050.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#define TAG "MPU6050"
#define I2C_PORT I2C_NUM_0
static uint8_t s_mpu_addr = 0x68;
static i2c_master_dev_handle_t dev_handle;
#define REG_WHO_AM_I      0x75
#define REG_PWR_MGMT_1    0x6B
#define REG_ACCEL_ZOUT_H  0x3F
#define MPU_WHO_AM_I_VAL  0x68

static esp_err_t i2c_write(uint8_t reg, uint8_t data)
{
    uint8_t buf[2] = {reg, data};
    // Dùng hàm TRANSMIT thay vì RM_DEVICE
    return i2c_master_transmit(dev_handle, buf, 2, pdMS_TO_TICKS(100));
}

static esp_err_t i2c_read(uint8_t reg, uint8_t *data, size_t len)
{
    // Gửi địa chỉ thanh ghi rồi nhận dữ liệu về
    // Dùng hàm TRANSMIT_RECEIVE
    return i2c_master_transmit_receive(dev_handle, &reg, 1, data, len, pdMS_TO_TICKS(100));
}

static esp_err_t mpu6050_probe_address(uint8_t addr)
{
    uint8_t who = 0;
    s_mpu_addr = addr;
    esp_err_t err = i2c_read(REG_WHO_AM_I, &who, 1);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "MPU6050 probe failed at 0x%02X: %s", addr, esp_err_to_name(err));
        return err;
    }
    if (who != MPU_WHO_AM_I_VAL) {
        ESP_LOGW(TAG, "MPU6050 wrong WHO_AM_I 0x%02X at addr 0x%02X", who, addr);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "MPU6050 found at I2C addr 0x%02X", addr);
    return ESP_OK;
}

esp_err_t mpu6050_init(i2c_master_bus_handle_t bus_handle) 
{
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x68, // Địa chỉ MPU6050
        .scl_speed_hz = 400000,
    };

    // Đăng ký thiết bị để lấy dev_handle
    esp_err_t err = i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle);
    if (err != ESP_OK) return err;

    // Sau đó mới thực hiện Wake up như bình thường
    return i2c_write(REG_PWR_MGMT_1, 0x00);
}

esp_err_t mpu6050_read_acc_z(int16_t *acc_z)
{
    uint8_t buf[2];
    esp_err_t err = i2c_read(REG_ACCEL_ZOUT_H, buf, 2);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Read accel failed at addr 0x%02X: %s", s_mpu_addr, esp_err_to_name(err));
        return err;
    }

    *acc_z = (int16_t)((buf[0] << 8) | buf[1]);
    return ESP_OK;
}
