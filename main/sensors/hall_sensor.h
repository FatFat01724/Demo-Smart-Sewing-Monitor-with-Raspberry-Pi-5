#ifndef HALL_SENSOR_H
#define HALL_SENSOR_H

#include <stdint.h>

void hall_init(void);

/* Tổng số mũi (debug / thống kê) */
uint32_t hall_get_count(void);
void hall_reset_count(void);

/* 🔥 API dùng cho EDGE */
uint8_t hall_get_and_clear(void);

#endif
