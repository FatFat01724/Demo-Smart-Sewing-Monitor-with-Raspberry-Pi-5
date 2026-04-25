#include "eeprom_24c02.h"
#include "driver/i2c.h"

#define EEPROM_ADDR 0x50
#define I2C_PORT    I2C_NUM_0

uint16_t eeprom_read_machine_id(void)
{
    uint8_t offset = 0x00;
    uint8_t data[2];

    i2c_master_write_read_device(
        I2C_PORT,
        EEPROM_ADDR,
        &offset, 1,
        data, 2,
        pdMS_TO_TICKS(100)
    );

    return (data[0] << 8) | data[1];
}
