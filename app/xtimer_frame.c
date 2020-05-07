
#include "xtimer_frame.h"

#include <stdio.h>
#include <string.h>

#define USE_RINGBUFF (0)
#define USE_MSG (1)

#if USE_MSG
#include "msg.h"
#else
#include "event.h"
#include "event/timeout.h"
#endif

#if USE_RINGBUFF
#include "ringbuffer_v2.h"

#define PAYLOAD_NUM (1)
#define PAYLOAD_LEN (150)

ringbuffer_v2_t frame_data_pending_queue;
char frame_data_pending_queue_buf[PAYLOAD_LEN * PAYLOAD_NUM];
unsigned int frame_data_pending_queue_data_real_len_list[PAYLOAD_NUM];
#endif

#if USE_MSG
#define TIMER_MSG_NUM (8)
static msg_t _msg_q[TIMER_MSG_NUM];
static msg_t timer_out_msg;
#else
static event_queue_t queue;
static event_t event;
#endif

kernel_pid_t frame_recv_pid;
#define FRAME_RECV_PRIORITY (6)
#define FRAME_RECV_STACKSIZE (1024)
char frame_recv_stack[FRAME_RECV_STACKSIZE];

xtimer_frame_t g_xtimer_frame;

void timer_timeout_handler(void *param)
{
    (void)param;

#if USE_MSG
    msg_send_int(&timer_out_msg, frame_recv_pid);
#else
    event_post(&queue, &event);
#endif
}

void rx_cb(void *arg, uint8_t data)
{
    (void)arg;
#if USE_RINGBUFF
    ringbuffer_v2_add_one(&frame_data_pending_queue, (char *)&data, 1);
#else
    g_xtimer_frame.frame_cache[g_xtimer_frame.frame_cache_len++] = data;
#endif
    xtimer_set(&g_xtimer_frame.frame_xtimer, g_xtimer_frame.frame_char_interval);
}

void xtimer_frame_init(void)
{
    g_xtimer_frame.frame_xtimer.callback = timer_timeout_handler;

    uart_init(g_xtimer_frame.frame_uart, g_xtimer_frame.frame_uart_baudrate, rx_cb, NULL);

#if USE_RINGBUFF
    ringbuffer_v2_init(&frame_data_pending_queue, frame_data_pending_queue_buf, PAYLOAD_LEN, PAYLOAD_NUM, frame_data_pending_queue_data_real_len_list);
#endif
    memset(g_xtimer_frame.frame_cache, 0, sizeof(g_xtimer_frame.frame_cache));

}

void xtimer_frame_parse_setup(uint8_t uart_dev, uint32_t uart_baudrate, uint32_t frame_char_interval, frame_recv_handler_t frame_recv_handler)
{
    g_xtimer_frame.frame_uart = uart_dev;
    g_xtimer_frame.frame_uart_baudrate = uart_baudrate;
    g_xtimer_frame.frame_char_interval = frame_char_interval;
    g_xtimer_frame.frame_recv_handler = frame_recv_handler;
}

void *_frame_receive_service(void *arg)
{
    (void)arg;
#if USE_MSG
    msg_t int_msg;
    msg_init_queue(_msg_q, TIMER_MSG_NUM);
#else
    event_queue_init(&queue);
#endif
    xtimer_frame_init();

    while (1)
    {
#if USE_MSG
        msg_receive(&int_msg);
        puts("receive message...");
#else
        event_wait(&queue);
        puts("receive event...");
#endif
#if USE_RINGBUFF
        ringbuffer_v2_get_one(&frame_data_pending_queue, g_xtimer_frame.frame_cache, &g_xtimer_frame.frame_cache_len);
#endif
        g_xtimer_frame.frame_recv_handler((uint8_t *)g_xtimer_frame.frame_cache, g_xtimer_frame.frame_cache_len);
        memset(g_xtimer_frame.frame_cache, 0, sizeof(g_xtimer_frame.frame_cache));
        g_xtimer_frame.frame_cache_len = 0;
    }

    return NULL;
}

void frame_recv_init(uint8_t uart_dev, uint32_t uart_baudrate, uint32_t frame_char_interval, frame_recv_handler_t frame_recv_handler)
{
    xtimer_frame_parse_setup(uart_dev, uart_baudrate, frame_char_interval, frame_recv_handler);

    frame_recv_pid = thread_create(frame_recv_stack, sizeof(frame_recv_stack),
                                   FRAME_RECV_PRIORITY, THREAD_CREATE_STACKTEST,
                                   _frame_receive_service, NULL, "_frame_receive_service");
}
