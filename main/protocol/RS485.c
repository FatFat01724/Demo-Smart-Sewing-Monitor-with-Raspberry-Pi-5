#include <stdio.h>
#include "RS485.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

static const char *TAG = "RS485";

/**
 * @brief Calculate checksum for packet
 */
static uint16_t calculate_checksum(const rs485_packet_t *packet)
{
    uint16_t checksum = 0;
    uint8_t *data = (uint8_t *)packet;
    
    // Calculate checksum for all bytes except checksum field
    for (int i = 0; i < sizeof(rs485_packet_t) - sizeof(uint16_t); i++) {
        checksum += data[i];
    }
    
    return checksum;
}

/**
 * @brief Initialize RS485 UART
 */
void rs485_init(void)
{
    // Configure UART parameters
    uart_config_t uartConfig = {
        .baud_rate = RS485_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0
    };

    // Apply UART config
    uart_param_config(RS485_UART_PORT, &uartConfig);

    // Set UART pins
    uart_set_pin(RS485_UART_PORT, RS485_TXD_PIN, RS485_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Install UART driver
    uart_driver_install(RS485_UART_PORT, RS485_RX_BUF_SIZE, RS485_TX_BUF_SIZE, 0, NULL, 0);

#if RS485_RTS_PIN >= 0
    // Configure RTS pin for RS485 control (high = transmit, low = receive)
    gpio_config_t gpioConfig = {
        .pin_bit_mask = (1ULL << RS485_RTS_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&gpioConfig);
    gpio_set_level(RS485_RTS_PIN, 0);  // Default to receive mode
    ESP_LOGI(TAG, "RS485 UART initialized on port %d with RTS pin %d", RS485_UART_PORT, RS485_RTS_PIN);
#else
    ESP_LOGI(TAG, "RS485 UART initialized on port %d without RTS, using auto-direction module", RS485_UART_PORT);
#endif
}

/**
 * @brief Send data via RS485
 */
void rs485_send_data(const rs485_packet_t *packet)
{
    rs485_packet_t tx_packet = *packet;
    
    // Calculate and set checksum
    tx_packet.checksum = calculate_checksum(&tx_packet);

#if RS485_RTS_PIN >= 0
    // Set RTS to transmit mode (high)
    gpio_set_level(RS485_RTS_PIN, 1);
    vTaskDelay(1 / portTICK_PERIOD_MS);
#endif

    // Send packet
    uart_write_bytes(RS485_UART_PORT, (const char *)&tx_packet, sizeof(rs485_packet_t));
    uart_wait_tx_done(RS485_UART_PORT, pdMS_TO_TICKS(100));

#if RS485_RTS_PIN >= 0
    // Set RTS to receive mode (low)
    gpio_set_level(RS485_RTS_PIN, 0);
#endif

    ESP_LOGD(TAG, "RS485 packet sent, sensor_id: %d", packet->sensor_id);
}

/**
 * @brief Read data from RS485
 */
int rs485_read_data(uint8_t *data, uint32_t length, uint32_t timeout_ms)
{
    int len = uart_read_bytes(RS485_UART_PORT, data, length, pdMS_TO_TICKS(timeout_ms));
    return len;
}

/**
 * @brief Send data via RS485 in text format (CSV)
 */
void rs485_send_data_text(const rs485_packet_t *packet, uint32_t packet_seq, const char *rtctime)
{
    char buffer[160];
    
    // Format: realtime,packet_seq,rung,hall,dong\r\n
    int len = snprintf(buffer, sizeof(buffer), "%s,%lu,%d,%d,%.2f\r\n",
                       rtctime,
                       packet_seq,
                       packet->data[0],
                       packet->data[1],
                       ((float)packet->data[2]) / 100.0f);
    
    if (len > 0) {
#if RS485_RTS_PIN >= 0
        // Set RTS to transmit mode (high)
        gpio_set_level(RS485_RTS_PIN, 1);
        vTaskDelay(1 / portTICK_PERIOD_MS);
#endif

        // Send text data
        uart_write_bytes(RS485_UART_PORT, buffer, len);
        uart_wait_tx_done(RS485_UART_PORT, pdMS_TO_TICKS(100));

#if RS485_RTS_PIN >= 0
        // Set RTS to receive mode (low)
        gpio_set_level(RS485_RTS_PIN, 0);
#endif
    }
    
    ESP_LOGD(TAG, "RS485 text packet sent");
}

/**
 * @brief Deinitialize RS485 UART
 */
void rs485_deinit(void)
{
    uart_driver_delete(RS485_UART_PORT);
#if RS485_RTS_PIN >= 0
    gpio_reset_pin(RS485_RTS_PIN);
#endif
    ESP_LOGI(TAG, "RS485 UART deinitialized");
}
