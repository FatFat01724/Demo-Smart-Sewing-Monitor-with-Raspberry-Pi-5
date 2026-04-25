#include "machine_info.h"
#include "eeprom_24c02.h"

static machine_info_t g_machine;

void machine_info_init(void)
{
    uint16_t raw_id = eeprom_read_machine_id();

    g_machine.machine_id = raw_id;
    g_machine.line_id = raw_id >> 8;
    g_machine.machine_type = raw_id & 0xFF;

    g_machine.stitch_count = 0;
    g_machine.rpm = 0;
    g_machine.is_running = false;
}

machine_info_t machine_info_get(void)
{
    return g_machine;
}

void machine_info_add_stitch(uint32_t delta)
{
    g_machine.stitch_count += delta;
}

void machine_info_set_rpm(uint32_t rpm)
{
    g_machine.rpm = rpm;
}

void machine_info_set_running(bool run)
{
    g_machine.is_running = run;
}
