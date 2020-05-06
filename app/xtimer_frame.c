
#include "xtimer.h"
#include "periph/uart.h"

#include <stdio.h>
#include <string.h>

#define USE_RINGBUFF (0)

#if USE_RINGBUFF
#include "ringbuffer_v2.h"

#define PAYLOAD_NUM (1)
#define PAYLOAD_LEN (150)

ringbuffer_v2_t frame_data_pending_queue;
char frame_data_pending_queue_buf[PAYLOAD_LEN * PAYLOAD_NUM];
unsigned int frame_data_pending_queue_data_real_len_list[PAYLOAD_NUM];
#endif

#define FRAME_UART_DEV (1)
#define FRAME_UART_BAUDRATE (115200)

static xtimer_t g_xtime;
static char frame_cache[256];
static unsigned frame_len = 0;

uint8_t receive_over_flag = 0;
void timer_timeout_handler(void *param)
{
    (void)param;

    receive_over_flag = 1;
}

void rx_cb(void *arg, uint8_t data)
{
    (void)arg;
    uint32_t char_interval = *((uint32_t *)arg);

#if USE_RINGBUFF
    ringbuffer_v2_add_one(&frame_data_pending_queue, (char *)&data, 1);
#else
    frame_cache[frame_len++] = data;
#endif

    xtimer_set(&g_xtime, char_interval);
}

uint32_t g_char_interval = 0;
void xtimer_frame_init(uint32_t char_interval)
{
    (void)char_interval;
    g_xtime.callback = timer_timeout_handler;

    g_char_interval = char_interval;

    uart_init(FRAME_UART_DEV, FRAME_UART_BAUDRATE, rx_cb, &g_char_interval);
#if USE_RINGBUFF
    ringbuffer_v2_init(&frame_data_pending_queue, frame_data_pending_queue_buf, PAYLOAD_LEN, PAYLOAD_NUM, frame_data_pending_queue_data_real_len_list);
#else
    memset(frame_cache, 0, sizeof(frame_cache));
#endif
}

void wait_print_frame(void)
{
    if (receive_over_flag)
    {
#if USE_RINGBUFF
        ringbuffer_v2_get_one(&frame_data_pending_queue, frame_cache, &frame_len);
#endif
        printf("receve frame len(%d) is :\r\n", frame_len);
        for (uint32_t i = 0; i < frame_len; i++)
        {
            printf("%02x ", frame_cache[i]);
        }
        printf("\r\n");
        memset(frame_cache, 0, sizeof(frame_cache));
        frame_len = 0;
        receive_over_flag = 0;
    }
}
