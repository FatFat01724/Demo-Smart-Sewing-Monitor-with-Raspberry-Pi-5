#pragma once
#include "driver/gpio.h"

void bsp_gpio_input_init(gpio_num_t pin);
void bsp_gpio_output_init(gpio_num_t pin);
