#pragma once
#include <stdint.h>

typedef struct {
    uint16_t machine_id;
    uint32_t time_ms;
    uint32_t stitch;
    float current;
    int16_t acc_z;
} machine_data_t;

void data_collector_task(void *arg);
