#ifndef BSP_I2C_H
#define BSP_I2C_H

#include "esp_err.h"
#include "driver/i2c_master.h"

esp_err_t bsp_i2c_init(void);
i2c_master_bus_handle_t bsp_i2c_get_bus_handle(void);

esp_err_t bsp_i2c_rtc_init(void);
i2c_master_bus_handle_t bsp_i2c_get_rtc_bus_handle(void);

#endif
