#ifndef DS3132_H
#define DS3132_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DS3132_I2C_ADDRESS 0x68

typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day_of_week;
    uint8_t day;
    uint8_t month;
    uint8_t year;
} ds3132_datetime_t;

esp_err_t ds3132_init(i2c_master_bus_handle_t bus_handle);
esp_err_t ds3132_get_time(ds3132_datetime_t *datetime);
esp_err_t ds3132_set_time(const ds3132_datetime_t *datetime);
void ds3132_format_datetime(const ds3132_datetime_t *datetime, char *buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif // DS3132_H
