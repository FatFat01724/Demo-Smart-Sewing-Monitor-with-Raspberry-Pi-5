#include <stdio.h>
#include "USB.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "USB";

/**
 * @brief Calculate checksum for packet
 */
static uint16_t calculate_checksum(const usb_packet_t *packet)
{
    uint16_t checksum = 0;
    uint8_t *data = (uint8_t *)packet;
    
    // Calculate checksum for all bytes except checksum field
    for (int i = 0; i < sizeof(usb_packet_t) - sizeof(uint16_t); i++) {
        checksum += data[i];
    }
    
    return checksum;
}

/**
 * @brief Initialize USB/CDC UART
 */
void usb_init(void)
{
    // Configure UART parameters
    uart_config_t uartConfig = {
        .baud_rate = USB_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0
    };

    // Apply UART config
    uart_param_config(USB_UART_PORT, &uartConfig);

    // Set UART pins (USB is preset, but we set explicitly)
    uart_set_pin(USB_UART_PORT, USB_TXD_PIN, USB_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Install UART driver
    uart_driver_install(USB_UART_PORT, USB_RX_BUF_SIZE, USB_TX_BUF_SIZE, 0, NULL, 0);

    ESP_LOGI(TAG, "USB UART initialized on port %d", USB_UART_PORT);
}

/**
 * @brief Send data via USB
 */
void usb_send_data(const usb_packet_t *packet)
{
    usb_packet_t tx_packet = *packet;
    
    // Calculate and set checksum
    tx_packet.checksum = calculate_checksum(&tx_packet);

    // Send packet
    uart_write_bytes(USB_UART_PORT, (const char *)&tx_packet, sizeof(usb_packet_t));
    uart_wait_tx_done(USB_UART_PORT, pdMS_TO_TICKS(100));

    ESP_LOGD(TAG, "USB packet sent, sensor_id: %d", packet->sensor_id);
}

/**
 * @brief Read data from USB
 */
int usb_read_data(uint8_t *data, uint32_t length, uint32_t timeout_ms)
{
    int len = uart_read_bytes(USB_UART_PORT, data, length, pdMS_TO_TICKS(timeout_ms));
    return len;
}

/**
 * @brief Send data via USB in text format (CSV)
 */
void usb_send_data_text(const usb_packet_t *packet)
{
    char buffer[128];
    
    // Format: timestamp,sensor_id,data0,data1,data2,data3\r\n
    int len = snprintf(buffer, sizeof(buffer), "%lu,%d,%d,%d,%d,%d\r\n",
                       packet->timestamp,
                       packet->sensor_id,
                       packet->data[0],
                       packet->data[1],
                       packet->data[2],
                       packet->data[3]);
    
    if (len > 0) {
        uart_write_bytes(USB_UART_PORT, buffer, len);
        uart_wait_tx_done(USB_UART_PORT, pdMS_TO_TICKS(100));
    }
    
    ESP_LOGD(TAG, "USB text packet sent");
}

/**
 * @brief Deinitialize USB UART
 */
void usb_deinit(void)
{
    uart_driver_delete(USB_UART_PORT);
    ESP_LOGI(TAG, "USB UART deinitialized");
}
