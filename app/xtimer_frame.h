#ifndef _XTIMER_FRAME
#define _XTIMER_FRAME

#include <stdint.h>

typedef int(*frame_recv_handler_t)(uint8_t *data, uint16_t len);

#define FRAME_UART_DEV (1)
#define FRAME_UART_BAUDRATE (115200)

#define CHAR_INTERVAL (2 * 1000) //2ms
#define TIMER_MSG_NUM (8)

void frame_recv_init(frame_recv_handler_t frame_recv_handler);

#endif
