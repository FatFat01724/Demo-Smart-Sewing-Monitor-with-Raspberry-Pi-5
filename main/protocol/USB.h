#ifndef USB_H
#define USB_H

#include <stdint.h>
#include "driver/uart.h"

// USB/CDC UART port
#define USB_UART_PORT UART_NUM_1
#define USB_TXD_PIN 8
#define USB_RXD_PIN 9

// Baud rate
#define USB_BAUD_RATE 115200

// Buffer size
#define USB_RX_BUF_SIZE 1024
#define USB_TX_BUF_SIZE 1024

typedef struct {
    uint32_t timestamp;
    uint16_t sensor_id;
    int16_t data[4];
    uint16_t checksum;
} usb_packet_t;

// Function prototypes
void usb_init(void);
void usb_send_data(const usb_packet_t *packet);
void usb_send_data_text(const usb_packet_t *packet);
int usb_read_data(uint8_t *data, uint32_t length, uint32_t timeout_ms);
void usb_deinit(void);

#endif // USB_H
