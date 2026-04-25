#include "acs712.h"
#include "bsp_adc.h"
#include "esp_log.h"

#define ACS_CH ADC_CHANNEL_5
#define TAG "ACS712"

// Ngưỡng để phát hiện cảm biến không kết nối
// Khi floating, ADC đọc giá trị rất cao (gần 4095) hoặc rất thấp (gần 0)
#define ADC_MIN_VALID    50    // Giá trị ADC tối thiểu hợp lệ
#define ADC_MAX_VALID    4000  // Giá trị ADC tối đa hợp lệ

void acs712_init(void)
{
    bsp_adc_init(ACS_CH);
}

float acs712_read_current(void)
{
    int raw = bsp_adc_read(ACS_CH);
    // Kiểm tra cảm biến có kết nối không
    if (raw < ADC_MIN_VALID || raw > ADC_MAX_VALID) {
        ESP_LOGW(TAG, "ADC raw=%d - Cảm biến không kết nối hoặc lỗi!", raw);
        return 0.0f;  // Trả về 0A khi không có cảm biến
    }
    
    float v = raw * 3.3f / 4095.0f;
    float current = (v - 2.5f) / 0.185f;
    
    // Giới hạn giá trị hợp lý (ACS712-20A đo từ -20A đến +20A)
    if (current < -20.0f) current = -20.0f;
    if (current > 20.0f) current = 20.0f;
    
    return current;
}
