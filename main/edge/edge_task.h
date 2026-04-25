#ifndef EDGE_TASK_H
#define EDGE_TASK_H

#include <stdint.h>
#include <stdbool.h>

void edge_task_init(void);
void edge_task_update(int hall, float current, int16_t acc_z, uint32_t time_ms);

uint32_t edge_get_stitch_delta(void);
uint32_t edge_get_rpm(void);
bool     edge_is_running(void);

#endif
