#include "ds3132.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static const char *TAG = "DS3132";
static i2c_master_dev_handle_t s_dev_handle;

static uint8_t bcd_to_bin(uint8_t val)
{
    return (val & 0x0F) + ((val >> 4) * 10);
}

static uint8_t bin_to_bcd(uint8_t val)
{
    return ((val / 10) << 4) | (val % 10);
}

static esp_err_t ds3132_i2c_read(uint8_t reg, uint8_t *data, size_t len)
{
    return i2c_master_transmit_receive(s_dev_handle, &reg, 1, data, len, pdMS_TO_TICKS(100));
}

static esp_err_t ds3132_i2c_write(uint8_t reg, const uint8_t *data, size_t len)
{
    if (len + 1 > 16) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t buf[16];
    buf[0] = reg;
    memcpy(&buf[1], data, len);
    return i2c_master_transmit(s_dev_handle, buf, len + 1, pdMS_TO_TICKS(100));
}

esp_err_t ds3132_init(i2c_master_bus_handle_t bus_handle)
{
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = DS3132_I2C_ADDRESS,
        .scl_speed_hz = 400000,
    };

    esp_err_t err = i2c_master_bus_add_device(bus_handle, &dev_cfg, &s_dev_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add DS3132 device: %s", esp_err_to_name(err));
        return err;
    }

    uint8_t seconds = 0;
    err = ds3132_i2c_read(0x00, &seconds, 1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to probe DS3132: %s", esp_err_to_name(err));
        return err;
    }

    // Clear CH bit so oscillator runs if it was stopped
    if (seconds & 0x80) {
        seconds &= ~0x80;
        err = ds3132_i2c_write(0x00, &seconds, 1);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to start DS3132 oscillator: %s", esp_err_to_name(err));
            return err;
        }
    }

    ESP_LOGI(TAG, "DS3132 RTC initialized at I2C address 0x%02X", DS3132_I2C_ADDRESS);
    return ESP_OK;
}

esp_err_t ds3132_get_time(ds3132_datetime_t *datetime)
{
    if (datetime == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t buf[7];
    esp_err_t err = ds3132_i2c_read(0x00, buf, sizeof(buf));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "DS3132 read failed: %s", esp_err_to_name(err));
        return err;
    }

    datetime->sec = bcd_to_bin(buf[0] & 0x7F);
    datetime->min = bcd_to_bin(buf[1] & 0x7F);

    uint8_t hours = buf[2];
    if (hours & 0x40) {
        // 12-hour mode
        bool pm = (hours & 0x20) != 0;
        uint8_t hour12 = bcd_to_bin(hours & 0x1F);
        datetime->hour = hour12 % 12 + (pm ? 12 : 0);
    } else {
        // 24-hour mode
        datetime->hour = bcd_to_bin(hours & 0x3F);
    }

    datetime->day_of_week = bcd_to_bin(buf[3] & 0x07);
    datetime->day = bcd_to_bin(buf[4] & 0x3F);
    datetime->month = bcd_to_bin(buf[5] & 0x1F);
    datetime->year = bcd_to_bin(buf[6]);

    return ESP_OK;
}

esp_err_t ds3132_set_time(const ds3132_datetime_t *datetime)
{
    if (datetime == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (datetime->sec > 59 || datetime->min > 59 || datetime->hour > 23 || datetime->day < 1 || datetime->day > 31 || datetime->month < 1 || datetime->month > 12) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t buf[7];
    buf[0] = bin_to_bcd(datetime->sec) & 0x7F;
    buf[1] = bin_to_bcd(datetime->min) & 0x7F;
    buf[2] = bin_to_bcd(datetime->hour) & 0x3F;
    buf[3] = bin_to_bcd(datetime->day_of_week % 8);
    buf[4] = bin_to_bcd(datetime->day) & 0x3F;
    buf[5] = bin_to_bcd(datetime->month) & 0x1F;
    buf[6] = bin_to_bcd(datetime->year);

    esp_err_t err = ds3132_i2c_write(0x00, buf, sizeof(buf));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "DS3132 write failed: %s", esp_err_to_name(err));
    }
    return err;
}

void ds3132_format_datetime(const ds3132_datetime_t *datetime, char *buffer, size_t buffer_size)
{
    if (buffer == NULL || buffer_size == 0 || datetime == NULL) {
        return;
    }

    snprintf(buffer, buffer_size,
             "%04u-%02u-%02u %02u:%02u:%02u",
             2000u + datetime->year,
             datetime->month,
             datetime->day,
             datetime->hour,
             datetime->min,
             datetime->sec);
}
