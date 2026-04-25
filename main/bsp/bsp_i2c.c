#include "bsp_i2c.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

#define TAG "BSP_I2C"

#define I2C_PORT       I2C_NUM_0
#define I2C_SDA_PIN    4      // SDA pin for main I2C bus
#define I2C_SCL_PIN    5      // SCL pin for main I2C bus
#define I2C_RTC_PORT   I2C_NUM_1
#define I2C_RTC_SDA_PIN 8     // SDA pin for DS3132 RTC
#define I2C_RTC_SCL_PIN 9     // SCL pin for DS3132 RTC
#define I2C_FREQ_HZ    400000

static i2c_master_bus_handle_t i2c_bus_handle = NULL;
static i2c_master_bus_handle_t i2c_rtc_bus_handle = NULL;

esp_err_t bsp_i2c_init(void)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_PORT,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus_handle));

    ESP_LOGI(TAG, "I2C main bus initialized");
    return ESP_OK;
}

i2c_master_bus_handle_t bsp_i2c_get_bus_handle(void)
{
    return i2c_bus_handle;
}

esp_err_t bsp_i2c_rtc_init(void)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_RTC_PORT,
        .sda_io_num = I2C_RTC_SDA_PIN,
        .scl_io_num = I2C_RTC_SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_rtc_bus_handle));

    ESP_LOGI(TAG, "I2C RTC bus initialized on SDA=%d SCL=%d", I2C_RTC_SDA_PIN, I2C_RTC_SCL_PIN);
    return ESP_OK;
}

i2c_master_bus_handle_t bsp_i2c_get_rtc_bus_handle(void)
{
    return i2c_rtc_bus_handle;
}
