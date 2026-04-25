#pragma once
#include "esp_adc/adc_oneshot.h"

void bsp_adc_init(adc_channel_t ch);
int  bsp_adc_read(adc_channel_t ch);
