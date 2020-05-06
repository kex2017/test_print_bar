#include "lora_io.h"

#include "xtimer.h"
#include "ringbuffer_v2.h"

#include "net/netdev.h"
#include "net/netdev/lora.h"
#include "net/lora.h"

#include "sx127x_internal.h"
#include "sx127x_params.h"
#include "sx127x_netdev.h"
#include "sx127x_registers.h"
#include "sx127x.h"

#include "periph/rtc.h"
#include "dev_cfg.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#define LORA_IO_PAYLOAD_NUM 2

#define LORA_IO_SERV_PRIORITY 5
#define LORA_IO_SERV_STACKSIZE 1024

#define LORA_IO_RECV_PRIORITY 6
#define LORA_IO_RECV_STACKSIZE (1024*2)

#define SX127X_LORA_MSG_QUEUE   (16U)
#define MSG_TYPE_ISR            (0x3456)

static uint32_t check_lora_cfg_last_time = 0;
static uint32_t check_lora_cfg_now_time = 0;
#define CHECK_LORA_CFG_INTERVAL (60)

void interval_check_lora_cfg(void)
{
	check_lora_cfg_now_time = rtc_get_counter();
	if(check_lora_cfg_now_time - check_lora_cfg_last_time >= CHECK_LORA_CFG_INTERVAL){
		check_lora_cfg_last_time = check_lora_cfg_now_time;
		lora_check_bw_sf_cr();
	}
}

typedef struct {
	sx127x_t sx127x;
	volatile int o_chan;
	volatile int i_chan;
	uint8_t bw;
	uint8_t sf;
	uint8_t cr;

	ringbuffer_v2_t out_data_pending_queue;
	char out_data_pending_queue_buf[LORA_IO_PAYLOAD_LEN * LORA_IO_PAYLOAD_NUM];
	unsigned int out_data_pending_queue_data_real_len_list[LORA_IO_PAYLOAD_NUM];

	char in_data_buf[LORA_IO_PAYLOAD_LEN];
	lora_io_recv_handler_t lora_io_recv_handler;

	kernel_pid_t lora_io_serv_pid;
	char lora_io_serv_stack[LORA_IO_SERV_STACKSIZE];

	kernel_pid_t lora_io_recv_pid;
	char lora_io_recv_stack[LORA_IO_RECV_STACKSIZE];

}lora_io_t;

static lora_io_t lora_io;

typedef enum {
	DATA_IN,
	DATA_OUT,
}lora_io_stream_type_t;

static void dump_lora_io_data_to_hex(uint8_t* data, uint16_t len,lora_io_stream_type_t stream_type)
{
	if(ENABLE_DEBUG)
	{
		int i = 0;
		if(stream_type == DATA_IN)
		{
			DEBUG("[lora io]: HEX IN = ");
		}else
		{
			DEBUG("[lora io]: HEX OUT = ");
		}
		for(i=0;i<len;i++)
		{
			DEBUG("%02x ",data[i]);
		}
		DEBUG("\r\n");
	}
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
            DEBUG("[lora io]: unexpected msg type\r\n");
        }
    }
}

void lora_io_recv_init(void)
{
    lora_io.lora_io_recv_pid = thread_create(lora_io.lora_io_recv_stack, sizeof(lora_io.lora_io_recv_stack),
    		LORA_IO_RECV_PRIORITY,THREAD_CREATE_STACKTEST, _recv_thread, NULL,"lora_io_recv");

    if (lora_io.lora_io_recv_pid <= KERNEL_PID_UNDEF)
    {
        DEBUG("[lora io]: creation of lora io recv thread failed\r\n");
    }
}

void lora_set_bw_sf_cr(void)
{
	netdev_t *netdev = (netdev_t *)&lora_io.sx127x;
	uint8_t lora_bw;
	int bw = lora_io.bw;
	switch (bw)
	{
		case 125:
			lora_bw = LORA_BW_125_KHZ;
			break;
		case 250:
			lora_bw = LORA_BW_250_KHZ;
			break;
		case 500:
			lora_bw = LORA_BW_500_KHZ;
			break;
		default:
			lora_bw = LORA_BW_125_KHZ;
			DEBUG("[lora io]: setup: invalid bandwidth value given, "
				 "only 125, 250 or 500 allowed.\r\n");
	}
	netdev->driver->set(netdev, NETOPT_BANDWIDTH,&lora_bw, sizeof(lora_bw));

    if (lora_io.sf < 7 || lora_io.sf > 12)
    {
    	lora_io.sf = 7;
        DEBUG("[lora io]: setup: invalid spreading factor value given set sf 7 default\r\n");
    }

    netdev->driver->set(netdev, NETOPT_SPREADING_FACTOR,&lora_io.sf, sizeof(lora_io.sf));

    if (lora_io.cr < 5 || lora_io.cr > 8)
	{
    	lora_io.cr = 5;
    	DEBUG("[lora io]: setup: invalid coding rate value given set 5 default\r\n");
	}
   uint8_t lora_cr = (uint8_t)(lora_io.cr - 4);
   netdev->driver->set(netdev, NETOPT_CODING_RATE,&lora_cr, sizeof(lora_cr));

}

uint8_t lora_check_bw_sf_cr(void)
{
	netdev_t *netdev = (netdev_t *)&lora_io.sx127x;
	uint8_t error = 0;

	uint8_t set_lora_bw, get_lora_bw;
	uint8_t set_lora_sf, get_lora_sf;
	uint8_t set_lora_cr, get_lora_cr;

	int bw = lora_io.bw;
	switch (bw)
	{
		case 125:
			set_lora_bw = LORA_BW_125_KHZ;
			break;
		case 250:
			set_lora_bw = LORA_BW_250_KHZ;
			break;
		case 500:
			set_lora_bw = LORA_BW_500_KHZ;
			break;
		default:
			set_lora_bw = LORA_BW_125_KHZ;
			DEBUG("[lora io]: check: invalid bandwidth value given, "
				"only 125, 250 or 500 allowed.\r\n");
	}
	netdev->driver->get(netdev, NETOPT_BANDWIDTH,&get_lora_bw, sizeof(get_lora_bw));
	if(get_lora_bw != set_lora_bw){
		error |= BW_ERROR_MASK;
		DEBUG("[lora io]: check: read back bw value is wrong!\r\n");
		netdev->driver->set(netdev, NETOPT_BANDWIDTH,&set_lora_bw, sizeof(set_lora_bw));
	}

	set_lora_sf = lora_io.sf;
	if (set_lora_sf < 7 || set_lora_sf > 12)
    {
    	set_lora_sf = 7;
        DEBUG("[lora io]: check: invalid spreading factor value given set sf 7 default\r\n");
    }
    netdev->driver->get(netdev, NETOPT_SPREADING_FACTOR,&get_lora_sf, sizeof(get_lora_sf));
	if(get_lora_sf != set_lora_sf){
		error |= SF_ERROR_MASK;
		DEBUG("[lora io]: check: read back sf value is wrong!\r\n");
		netdev->driver->set(netdev, NETOPT_SPREADING_FACTOR,&set_lora_sf, sizeof(set_lora_sf));
	}

	set_lora_cr = lora_io.cr;
	if (set_lora_cr < 5 || set_lora_cr > 8)
	{
    	set_lora_cr = 5;
    	DEBUG("[lora io]: check: invalid coding rate value given set 5 default\r\n");
	}
	set_lora_cr = (set_lora_cr - 4);
    netdev->driver->get(netdev, NETOPT_CODING_RATE,&get_lora_cr, sizeof(get_lora_cr));
	if(get_lora_cr != set_lora_cr)
	{
		error |= CR_ERROR_MASK;
		DEBUG("[lora io]: check: read back cr value is wrong!\r\n");
		netdev->driver->set(netdev, NETOPT_CODING_RATE,&set_lora_cr, sizeof(set_lora_cr));
	}
	if(error){
		record_lora_error_info(rtc_get_counter(), error);
	}
	else
	{
		DEBUG("[lora io]: check:lora cfg is right\r\n");
	}

	return error;
}

void event_cb_for_sx127x(netdev_t *dev, netdev_event_t event)
{
    if (event == NETDEV_EVENT_ISR)
    {
        msg_t msg;

        msg.type = MSG_TYPE_ISR;
        msg.content.ptr = dev;
        if (msg_send(&msg, lora_io.lora_io_recv_pid) <= 0)
        {
            DEBUG("[lora io]: gnrc_netdev: possibly lost interrupt.\r\n");
        }
    }
    else
    {
        size_t len;
        netdev_lora_rx_info_t packet_info;
        switch (event)
        {
        case NETDEV_EVENT_RX_STARTED:
            DEBUG("[lora io]: data reception started\r\n");
            break;

        case NETDEV_EVENT_RX_COMPLETE:
            len = dev->driver->recv(dev, NULL, 0, 0);
            if(len >0 && len <= LORA_IO_PAYLOAD_LEN)
            {
            	dev->driver->recv(dev, lora_io.in_data_buf, len, &packet_info);
				DEBUG("[RSSI: %i, SNR: %i, TOA: %" PRIu32 "]\r\n",
						packet_info.rssi, (int)packet_info.snr,
							sx127x_get_time_on_air((const sx127x_t *)dev, len));
            	dump_lora_io_data_to_hex((uint8_t*)lora_io.in_data_buf,len,DATA_IN);
            	lora_io.lora_io_recv_handler((uint8_t*)lora_io.in_data_buf,len);
				increase_lora_recv_msg_num();
            }
            else
            {
            	DEBUG("[lora io]: recv data len(%d) overflow\r\n",len);
            }

            break;

        case NETDEV_EVENT_TX_COMPLETE:
            sx127x_set_sleep(&lora_io.sx127x);
            DEBUG("[lora io]: transmission completed\r\n");
            break;

        case NETDEV_EVENT_CAD_DONE:
        	DEBUG("[lora io]: cad done\r\n");
            break;

        case NETDEV_EVENT_TX_TIMEOUT:
            sx127x_set_sleep(&lora_io.sx127x);
            DEBUG("[lora io]: data tx timeout\r\n");
            break;

        default:
            DEBUG("[lora io]: unexpected netdev event received: %d\r\n", event);
            break;
        }
    }
}

void lora_init(void)
{
	lora_io.sx127x.params = sx127x_params[0];
	netdev_t *netdev = (netdev_t *)&lora_io.sx127x;
	netdev->driver = &sx127x_driver;
    if (netdev->driver->init(netdev) < 0)
    {
        DEBUG("[lora io]: init dev(%d):Failed to initialize SX127x device, exiting\r\n", lora_io.sx127x.params.spi);
    }

    netdev->event_callback = event_cb_for_sx127x;
    lora_set_bw_sf_cr();
}

void lora_set_channel(int channel)
{
    uint32_t chan = channel;
    netdev_t *netdev = (netdev_t *)&lora_io.sx127x;
    netdev->driver->set(netdev, NETOPT_CHANNEL_FREQUENCY, &chan,sizeof(chan));
}

void lora_set_listen_mode(void)
{
	lora_set_channel(lora_io.i_chan);

	netdev_t *netdev = (netdev_t *)&lora_io.sx127x;
	const netopt_enable_t single = false;
	netdev->driver->set(netdev, NETOPT_SINGLE_RECEIVE, &single, sizeof(single));
	const uint32_t timeout = 0;
	netdev->driver->set(netdev, NETOPT_RX_TIMEOUT, &timeout, sizeof(timeout));

	uint8_t state = NETOPT_STATE_RX;
	netdev->driver->set(netdev, NETOPT_STATE, &state, sizeof(state));
}

bool lora_set_syncword(uint8_t syncword)
{
	sx127x_set_syncword(&lora_io.sx127x, syncword);

	return (syncword == sx127x_get_syncword(&lora_io.sx127x));
}

bool is_channel_free(uint32_t freq, int16_t rssi_threshold)
{
    return sx127x_is_channel_free(&lora_io.sx127x, freq, rssi_threshold);
}

void lora_send(uint8_t *data,uint8_t len)
{
	lora_set_channel(lora_io.o_chan);
	    iolist_t iolist = {
	        .iol_base = data,
	        .iol_len = len};
	netdev_t *netdev = (netdev_t *)&lora_io.sx127x;
	if (netdev->driver->send(netdev, &iolist) == -ENOTSUP)
	{
		DEBUG("[lora io]: cannot send: radio is still transmitting\r\n");
	}
	increase_lora_send_msg_num();
}

void lora_io_send_data_from_out_data_pending_queue(void)
{
	char data[LORA_IO_PAYLOAD_LEN] = {0};
	unsigned len = 0;
	if(ringbuffer_v2_get_one(&lora_io.out_data_pending_queue,data,&len) < 0)
	{
		return;
	}
	dump_lora_io_data_to_hex((uint8_t*)data,len,DATA_OUT);
	lora_send((uint8_t*)data,len);
}

void *lora_io_serv(void *arg)
{
	(void)arg;
	// int retry_times = 0;

	lora_init();
	// lora_set_syncword(LORA_SYNCWORD_PRIVATE);
	lora_io_recv_init();
	lora_set_listen_mode();
	while (1)
	{
		lora_io_send_data_from_out_data_pending_queue();
		xtimer_usleep(50*1000);
		lora_set_listen_mode();
		xtimer_usleep(50*1000);
		interval_check_lora_cfg();
		// retry_times = 0;
    	// while((SX127X_RF_OPMODE_RECEIVER != sx127x_get_op_mode(&lora_io.sx127x) && retry_times < 30)){
        // 	xtimer_usleep(100*1000);
        // 	retry_times++;
    	// }
    	// if(retry_times == 30){
        // 	DEBUG("[lora io]: listen mode set error !retry time is %d\r\n", retry_times);
    	// }
	}
	return NULL;
}

void lora_io_setup(int i_chan,int o_chan,uint8_t bw,uint8_t sf,uint8_t cr,lora_io_recv_handler_t lora_io_recv_handler)
{
	lora_io.i_chan = i_chan;
	lora_io.o_chan = o_chan;
	lora_io.bw = bw;
	lora_io.sf = sf;
	lora_io.cr = cr;
	lora_io.lora_io_recv_handler = lora_io_recv_handler;

	lora_io.lora_io_serv_pid = KERNEL_PID_UNDEF;
	lora_io.lora_io_recv_pid = KERNEL_PID_UNDEF;
	ringbuffer_v2_init(&lora_io.out_data_pending_queue, lora_io.out_data_pending_queue_buf, LORA_IO_PAYLOAD_LEN,LORA_IO_PAYLOAD_NUM,lora_io.out_data_pending_queue_data_real_len_list);
}

void lora_io_serv_start(void)
{
	if (lora_io.lora_io_serv_pid == KERNEL_PID_UNDEF) {
		lora_io.lora_io_serv_pid = thread_create(lora_io.lora_io_serv_stack,
						sizeof(lora_io.lora_io_serv_stack),
						LORA_IO_SERV_PRIORITY, THREAD_CREATE_STACKTEST,
						lora_io_serv, NULL, "lora_io_serv");
	}
}

void lora_io_change_channel(int i_chan,int o_chan)
{
	lora_io.i_chan = i_chan;
	lora_io.o_chan = o_chan;
}

int lora_io_send(uint8_t* data,uint16_t len)
{
	return ringbuffer_v2_add_one(&lora_io.out_data_pending_queue, (char*)data,(unsigned)len);
}

