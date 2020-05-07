#include <stdio.h>

#include "event.h"
#include "event/timeout.h"

#define TIMEOUT         (50U * US_PER_MS)       /* 50ms */

static unsigned _wakeup_evt = 0;
static unsigned _wakeup_timeout = 0;

static void callback(event_t *arg);

static event_queue_t queue;
static event_t event = {.handler = callback};

static void callback(event_t *arg)
{
    (void)arg;
    printf("event happened...\r\n");
    ++_wakeup_evt;
}

void wait_for_event(void);
void *_cnt_thread(void *arg)
{
    (void)arg;
    event_queue_init(&queue);

    while (1) {
        wait_for_event();
        continue;
        event_t *evt = event_wait(&queue);
        // event_t *evt = event_wait_timeout(&queue, TIMEOUT);
        if (evt) {
            evt->handler(evt);
        }
        else {
            ++_wakeup_timeout;
        }
    }

    return NULL;
}

static char _stack[1024];
void test_event(void)
{
    puts("[START] event test application.\n");

    thread_create(_stack, sizeof(_stack), 6, 0, _cnt_thread, NULL, "cnt");

    for (unsigned i = 0; i < 3; i++)
    {
        event_post(&queue, &event);
        xtimer_usleep(5U * US_PER_MS);
    }

    // puts("launching event queue");
    // event_loop(&queue);
}

void creat_thread_event(void)
{
    thread_create(_stack, sizeof(_stack), 6, 0, _cnt_thread, NULL, "cnt");
}

void wait_for_event(void)
{
    // event_wait(&queue);
    event_t *evt = event_wait_timeout(&queue, 500 * 1000);
    if (evt) {
            evt->handler(evt);
        }
    else{
        printf("evt NULL!!!\r\n");
    }
}
