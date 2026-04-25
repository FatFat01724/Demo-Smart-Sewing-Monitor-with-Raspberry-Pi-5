#include "hall_sensor.h"
#include "bsp_gpio.h"
#include "driver/gpio.h"
#include "esp_attr.h"

#define HALL_GPIO GPIO_NUM_15

static volatile uint32_t stitch_count = 0;
static volatile uint8_t last_state = 1;   // giả sử ban đầu đèn OFF (pull-up)

/* ISR Hall */
static void IRAM_ATTR hall_isr(void* arg)
{
    uint8_t current_state = gpio_get_level(HALL_GPIO);

    /* Đếm 1 lần khi đèn vừa sáng xong và tắt (ON -> OFF) */
    if (last_state == 1 && current_state == 0)
    {
        stitch_count++;
    }

    last_state = current_state;
}

void hall_init(void)
{
    bsp_gpio_input_init(HALL_GPIO);

    /* Bắt cả cạnh lên và xuống */
    gpio_set_intr_type(HALL_GPIO, GPIO_INTR_ANYEDGE);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(HALL_GPIO, hall_isr, NULL);
}

uint32_t hall_get_count(void)
{
    return stitch_count;
}

void hall_reset_count(void)
{
    stitch_count = 0;
}

/* 🔥 HÀM DÙNG CHO EDGE */
uint8_t hall_get_and_clear(void)
{
    uint8_t ret = 0;

    if (stitch_count > 0)
    {
        ret = 1;            // có mũi trong chu kỳ
        stitch_count = 0;   // clear ngay
    }

    return ret;
}
