#ifndef MACHINE_INFO_H
#define MACHINE_INFO_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint16_t machine_id;
    uint8_t  line_id;
    uint8_t  machine_type;

    uint32_t stitch_count;
    uint32_t rpm;
    bool     is_running;
} machine_info_t;

void machine_info_init(void);
machine_info_t machine_info_get(void);

void machine_info_add_stitch(uint32_t delta);
void machine_info_set_rpm(uint32_t rpm);
void machine_info_set_running(bool run);

#endif
