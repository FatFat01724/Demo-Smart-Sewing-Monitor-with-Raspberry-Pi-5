idf_component_register(
    SRCS
        "DA2.c"

        "app/data_collector.c"
        "app/machine_info.c"

        "edge/edge_task.c"          

        "bsp/bsp_gpio.c"
        "bsp/bsp_adc.c"
        "bsp/bsp_i2c.c"

        "sensors/hall_sensor.c"
        "sensors/acs712.c"
        "drivers/mpu6050.c"
        "drivers/ds3132.c"
        "drivers/eeprom_24c02.c"

        "protocol/RS485.c"
        "protocol/USB.c"

    INCLUDE_DIRS
        "."
        "app"
        "edge"                    
        "bsp"
        "sensors"
        "drivers"
        "protocol"
)
