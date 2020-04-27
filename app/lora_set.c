#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "ringbuffer.h"
#include "isrpipe/read_timeout.h"
#include "thread.h"
#include "xtimer.h"
#include "shell.h"
#include "shell_commands.h"

#include "net/netdev.h"
#include "net/netdev/lora.h"
#include "net/lora.h"

#include "board.h"

#include "sx127x_internal.h"
#include "sx127x_params.h"
#include "sx127x_netdev.h"

// #include "data_transfer.h"
// #include "frame_paser.h"
#include "lora_set.h"
// #include "x_delay.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

#define SX127X_LORA_MSG_QUEUE   (16U)
#define SX127X_STACKSIZE        (THREAD_STACKSIZE_DEFAULT)

#define MSG_TYPE_ISR            (0x3456)

static char stack[SX127X_STACKSIZE];
static kernel_pid_t _recv_pid;

static uint8_t message[32];

sx127x_t sx127x;

kernel_pid_t caller_pid;

void lora_set_bw(sx127x_t sx127x,uint8_t lora_bw)
{
    netdev_t *netdev = (netdev_t *)&sx127x;
    int bw = lora_bw;
    switch (bw)
    {
    case 125:
        DEBUG("[lora] setup: setting 125KHz bandwidth\r\n");
        lora_bw = LORA_BW_125_KHZ;
        break;

    case 250:
        DEBUG("[lora] setup: setting 250KHz bandwidth\r\n");
        lora_bw = LORA_BW_250_KHZ;
        break;

    case 500:
        DEBUG("[lora] setup: setting 500KHz bandwidth\r\n");
        lora_bw = LORA_BW_500_KHZ;
        break;

    default:
        lora_bw = LORA_BW_125_KHZ;
        DEBUG("[lora] setup: invalid bandwidth value given, "
             "only 125, 250 or 500 allowed.\r\n");
    }
    netdev->driver->set(netdev, NETOPT_BANDWIDTH,&lora_bw, sizeof(lora_bw));
}

void lora_set_sf(sx127x_t sx127x,uint8_t lora_sf)
{
    netdev_t *netdev = (netdev_t *)&sx127x;

    if (lora_sf < 7 || lora_sf > 12)
    {
        lora_sf = LORA_DEFAULT_SF;
        DEBUG("[lora] setup: invalid spreading factor value given\r\n");
        DEBUG("[lora] setup: set spreading factor value for default\r\n");
    }
    DEBUG("[lora] setup: set spreading factor value as:%d\r\n", lora_sf);
    netdev->driver->set(netdev, NETOPT_SPREADING_FACTOR,&lora_sf, sizeof(lora_sf));
}

void lora_set_cr(sx127x_t sx127x, uint8_t cr)
{
    netdev_t *netdev = (netdev_t *)&sx127x;

    if (cr < 5 || cr > 8)
    {
        cr = LORA_DEFAULT_CR;
        DEBUG("[lora]setup: invalid coding rate value givenr\r\n");
        DEBUG("[lora]setup: set coding rate value for default\r\n");
    }
    uint8_t lora_cr = (uint8_t)(cr - 4);
    DEBUG("[lora]setup: set coding rate value as:%d\r\n", cr);
    netdev->driver->set(netdev, NETOPT_CODING_RATE,&lora_cr, sizeof(lora_cr));
}

void lora_setup(sx127x_t sx127x, uint8_t lora_bw, uint8_t lora_sf, uint8_t cr)
{
    lora_set_bw(sx127x,lora_bw);
    lora_set_sf(sx127x,lora_sf);
    lora_set_cr(sx127x,cr);
}

void lora_set_channel(sx127x_t sx127x,int channel)
{
    uint32_t chan = channel;
    netdev_t *netdev = (netdev_t *)&sx127x;
    netdev->driver->set(netdev, NETOPT_CHANNEL_FREQUENCY, &chan,
                        sizeof(chan));
    DEBUG("[lora] New channel set as:%dHz\r\n", channel);
}

// extern frame_paser_dev_t lora_fp_dev;
void _event_cb(netdev_t *dev, netdev_event_t event)
{
    if (event == NETDEV_EVENT_ISR)
    {
        msg_t msg;

        msg.type = MSG_TYPE_ISR;
        msg.content.ptr = dev;

        if (msg_send(&msg, _recv_pid) <= 0)
        {
            DEBUG("[lora] gnrc_netdev: possibly lost interrupt.\r\n");
        }
    }
    else
    {
        size_t len;
        netdev_lora_rx_info_t packet_info;
        switch (event)
        {
        case NETDEV_EVENT_RX_STARTED:
            DEBUG("[lora] Data reception started\r\n");
            break;

        case NETDEV_EVENT_RX_COMPLETE:
            len = dev->driver->recv(dev, NULL, 0, 0);
            dev->driver->recv(dev, message, len, &packet_info);
            DEBUG("[lora] {Payload: \"%s\" (%d bytes), RSSI: %i, SNR: %i, TOA: %" PRIu32 "}\r\n",
                   message, (int)len, packet_info.rssi, (int)packet_info.snr,
                   sx127x_get_time_on_air((const sx127x_t *)dev, len));

            // if (!is_frame_parser_busy(&lora_fp_dev))
            // {
            //     add_frame_data_stream(&lora_fp_dev, message, len);
            // }
            // for (int i = 0; i < (int)len; i++)
            // {
            //     printf("[lora] Payload[%d]: %02x \r\n",i,message[i]);
            // }

            break;

        case NETDEV_EVENT_TX_COMPLETE:
            sx127x_set_sleep(&sx127x);
            DEBUG("Transmission completed\r\n");
            break;

        case NETDEV_EVENT_CAD_DONE:
            break;

        case NETDEV_EVENT_TX_TIMEOUT:
            sx127x_set_sleep(&sx127x);
            break;

        default:
            DEBUG("[lora] Unexpected netdev event received: %d\r\n", event);
            break;
        }
    }
}

// extern frame_paser_dev_t uart_fp_dev;
// static void rx_cb(void *arg, uint8_t data)
// {
//     (void)arg;
//     uint8_t temp_data[1];
//     temp_data[0] = data;
//     if (!is_frame_parser_busy(&uart_fp_dev))
//     {
//         add_frame_data_stream(&uart_fp_dev, temp_data, sizeof(temp_data));
//     }

// }

// extern int lora_transfer_status;
int lora_send_message(uint8_t *message,uint8_t message_len)
{
    lora_set_channel(sx127x, SX127X_CHAN);
    // lora_send_timer_enable();

    iolist_t iolist = {
        .iol_base = message,
        .iol_len = message_len};

    netdev_t *netdev = (netdev_t *)&sx127x;
    if (netdev->driver->send(netdev, &iolist) == -ENOTSUP)
    {
        DEBUG("[lora] Cannot send: radio is still transmitting\r\n");
    }

    DEBUG("[lora] sending \"%s\" payload (%u bytes)\n", message, message_len);

    caller_pid = thread_getpid();

    /* Wait until the function receive some information */
    xtimer_usleep(100 * 1000);
    lora_listen_message();
    return 0;

    // lora_transfer_status = LORA_LISTEN;
    msg_t msg;
    msg_receive(&msg);
    return msg.type;
}

int lora_listen_message(void)
{
    lora_set_channel(sx127x, SX127X_CHAN);

    netdev_t *netdev = (netdev_t *)&sx127x;
    /* Switch to continuous listen mode */
    const netopt_enable_t single = false;
    netdev->driver->set(netdev, NETOPT_SINGLE_RECEIVE, &single, sizeof(single));
    const uint32_t timeout = 0;
    netdev->driver->set(netdev, NETOPT_RX_TIMEOUT, &timeout, sizeof(timeout));

    /* Switch to RX state */
    uint8_t state = NETOPT_STATE_RX;
    netdev->driver->set(netdev, NETOPT_STATE, &state, sizeof(state));
    netdev->driver->get(netdev, 5, &state, sizeof(state));
    // if(state == NETOPT_STATE_RX){
    //     DEBUG("[lora] Listen mode set\r\n");
    // }
    DEBUG("[lora] Listen mode set\r\n");
    return 0;
}

void *_recv_thread(void *arg)
{
    (void)arg;

    static msg_t _msg_q[SX127X_LORA_MSG_QUEUE];
    msg_init_queue(_msg_q, SX127X_LORA_MSG_QUEUE);

    while (1) {
        msg_t msg;
        msg_receive(&msg);
        if (msg.type == MSG_TYPE_ISR) {
            netdev_t *dev = msg.content.ptr;
            dev->driver->isr(dev);
        }
        else {
            DEBUG("Unexpected msg type\r\n");
        }
    }
    return NULL;
}

kernel_pid_t lora_recv_service_init(void)
{
    _recv_pid = thread_create(stack, sizeof(stack), THREAD_PRIORITY_MAIN - 1,
                              THREAD_CREATE_STACKTEST, _recv_thread, NULL,
                              "recv_thread");

    if (_recv_pid <= KERNEL_PID_UNDEF)
    {
        DEBUG("Creation of receiver thread failed\r\n");
        return 1;
    }
    return 0;
}

void lora_set_init(void)
{
    // uart_init(LORA_UART_DEV,LORA_UART_BAUDRATE,rx_cb,NULL);
    sx127x.params = sx127x_params[0];
    netdev_t *netdev = (netdev_t *)&sx127x;
    netdev->driver = &sx127x_driver;

    if (netdev->driver->init(netdev) < 0)
    {
        DEBUG("[lora] init:Failed to initialize SX127x device, exiting\r\n");
    }
    netdev->event_callback = _event_cb;

    lora_setup(sx127x, SX127X_BW, SX127X_SF, SX127X_CR);
    // lora_set_channel(sx127x,470000000);

    lora_recv_service_init();
}

