#include "bsp_adc.h"
#include "esp_log.h"

#define TAG "BSP_ADC"

static adc_oneshot_unit_handle_t adc_handle = NULL;

void bsp_adc_init(adc_channel_t ch)
{
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ch, &config));

    ESP_LOGI(TAG, "ADC initialized");
}

int bsp_adc_read(adc_channel_t ch)
{
    int adc_raw;
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ch, &adc_raw));
    return adc_raw;
}
