
#include "xtimer_frame.h"

#include "xtimer.h"
#include "periph/uart.h"
#include "msg.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define USE_RINGBUFF (0)

#if USE_RINGBUFF
#include "ringbuffer_v2.h"

#define PAYLOAD_NUM (1)
#define PAYLOAD_LEN (150)

ringbuffer_v2_t frame_data_pending_queue;
char frame_data_pending_queue_buf[PAYLOAD_LEN * PAYLOAD_NUM];
unsigned int frame_data_pending_queue_data_real_len_list[PAYLOAD_NUM];
#endif

static msg_t _msg_q[TIMER_MSG_NUM];

kernel_pid_t frame_recv_pid;

static xtimer_t g_xtime;
static char frame_cache[256];
static unsigned frame_len = 0;
static uint32_t g_char_interval = 0;

void timer_timeout_handler(void *param)
{
    (void)param;

    msg_t timer_out_msg;
    msg_send_int(&timer_out_msg, frame_recv_pid);
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

void xtimer_frame_init(uint32_t char_interval)
{
    g_xtime.callback = timer_timeout_handler;

    g_char_interval = char_interval;

    uart_init(FRAME_UART_DEV, FRAME_UART_BAUDRATE, rx_cb, &g_char_interval);
#if USE_RINGBUFF
    ringbuffer_v2_init(&frame_data_pending_queue, frame_data_pending_queue_buf, PAYLOAD_LEN, PAYLOAD_NUM, frame_data_pending_queue_data_real_len_list);
#else
    memset(frame_cache, 0, sizeof(frame_cache));
#endif
}

void *_frame_receive_service(void *arg)
{
    msg_t int_msg;

    frame_recv_handler_t frame_handler = (frame_recv_handler_t)arg;

    msg_init_queue(_msg_q, TIMER_MSG_NUM);
    xtimer_frame_init(CHAR_INTERVAL);

    while (1)
    {
        msg_receive(&int_msg);
#if USE_RINGBUFF
        ringbuffer_v2_get_one(&frame_data_pending_queue, frame_cache, &frame_len);
#endif
        frame_handler((uint8_t*)frame_cache, frame_len);

        memset(frame_cache, 0, sizeof(frame_cache));
        frame_len = 0;
    }

    return NULL;
}

#define FRAME_RECV_PRIORITY (6)
#define FRAME_RECV_STACKSIZE (1024)
char frame_recv_stack[FRAME_RECV_STACKSIZE];
void frame_recv_init(frame_recv_handler_t frame_recv_handler)
{
    frame_recv_pid = thread_create(frame_recv_stack, sizeof(frame_recv_stack),
                                   FRAME_RECV_PRIORITY, THREAD_CREATE_STACKTEST, _frame_receive_service, frame_recv_handler, "_frame_receive_service");
}
