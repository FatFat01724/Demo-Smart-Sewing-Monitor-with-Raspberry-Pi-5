#ifndef RS485_H
#define RS485_H

#include <stdint.h>
#include "driver/uart.h"

// RS485 UART port
#define RS485_UART_PORT UART_NUM_1
#define RS485_TXD_PIN 17
#define RS485_RXD_PIN 18
#define RS485_RTS_PIN -1  // RE/DE control pin; set to -1 for 4-pin auto-direction modules

// Baud rate
#define RS485_BAUD_RATE 115200

// Buffer size
#define RS485_RX_BUF_SIZE 1024
#define RS485_TX_BUF_SIZE 1024

// Data structure
typedef struct {
    uint32_t timestamp;
    uint16_t sensor_id;
    int16_t data[4];
    uint16_t checksum;
} rs485_packet_t;

// Function prototypes
void rs485_init(void);
void rs485_send_data(const rs485_packet_t *packet);
void rs485_send_data_text(const rs485_packet_t *packet, uint32_t packet_seq, const char *rtctime);
int rs485_read_data(uint8_t *data, uint32_t length, uint32_t timeout_ms);
void rs485_deinit(void);

#endif // RS485_H
