#ifndef _XTIMER_FRAME
#define _XTIMER_FRAME

#include <stdint.h>

#include "xtimer.h"
#include "periph/uart.h"

#define FRAME_CACHE_SIZE (256)

typedef int(*frame_recv_handler_t)(uint8_t *data, uint16_t len);

typedef struct _xtimer_frame{
    xtimer_t frame_xtimer;
    char frame_cache[FRAME_CACHE_SIZE];
    unsigned frame_cache_len;
    unsigned frame_char_interval;
    frame_recv_handler_t frame_recv_handler;

    uart_t frame_uart;
    uint32_t frame_uart_baudrate;
}xtimer_frame_t;

#define FRAME_UART_DEV (1)
#define FRAME_UART_BAUDRATE (115200)

// #define CHAR_INTERVAL (2 * 1000) //2ms
#define CHAR_INTERVAL (10 * 60 * 1000 * 1000) //2s

void xtimer_frame_parse_setup(uint8_t uart_dev, uint32_t uart_baudrate, uint32_t frame_char_interval, frame_recv_handler_t frame_recv_handler);

void frame_recv_init(uint8_t uart_dev, uint32_t uart_baudrate, uint32_t frame_char_interval, frame_recv_handler_t frame_recv_handler);

#endif
