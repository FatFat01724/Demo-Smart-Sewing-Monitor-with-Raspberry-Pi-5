#include "edge_task.h"

#define ACC_THRESHOLD     800
#define CURRENT_THRESHOLD 0.3f
#define COOLDOWN_MS       8
#define STOP_TIMEOUT_MS   300

static uint32_t last_stitch_time = 0;
static uint32_t stitch_delta = 0;
static uint32_t rpm = 0;
static bool running = false;

void edge_task_init(void)
{
    last_stitch_time = 0;
    stitch_delta = 0;
    rpm = 0;
    running = false;
}

void edge_task_update(int hall, float current, int16_t acc_z, uint32_t time_ms)
{
    if (hall &&
        current > CURRENT_THRESHOLD &&
        acc_z < -ACC_THRESHOLD &&
        (time_ms - last_stitch_time) > COOLDOWN_MS)
    {
        stitch_delta++;

        if (last_stitch_time > 0)
            rpm = 60000 / (time_ms - last_stitch_time);

        last_stitch_time = time_ms;
        running = true;
    }

    if ((time_ms - last_stitch_time) > STOP_TIMEOUT_MS)
    {
        running = false;
        rpm = 0;
    }
}

uint32_t edge_get_stitch_delta(void)
{
    uint32_t tmp = stitch_delta;
    stitch_delta = 0;
    return tmp;
}

uint32_t edge_get_rpm(void)
{
    return rpm;
}

bool edge_is_running(void)
{
    return running;
}
