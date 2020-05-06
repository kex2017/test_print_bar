#ifndef LORA_SET_H_
#define LORA_SET_H_

#include "sx127x_internal.h"
#include "sx127x_params.h"
#include "sx127x_netdev.h"

#include "periph/uart.h"
#include "isrpipe.h"
#include "ringbuffer.h"


#define LORA_DEFAULT_SF 7
#define LORA_DEFAULT_CR 5

#define LORA_UART_DEV UART_DEV(1)
#define LORA_UART_BAUDRATE (115200)
#define LORA_UART_RX_BUFFSIZE (128U)

#define SX127X_BW (125)//bandwidth
#define SX127X_SF (7)//spreading factor
#define SX127X_CR (5) //coding rate
#define SX127X_CHAN (480000000)

typedef struct {
    char rx_mem[LORA_UART_RX_BUFFSIZE];
    ringbuffer_t rx_buf;
} uart_rcv_t;


typedef struct
{
    uart_t uart;
    uint32_t baudrate;
    isrpipe_t isrpipe;
    uint8_t *isrpipe_buf;
    unsigned isrpipe_buf_size;
    uint8_t set_highest_priority;
} lora_uart_dev_t;

void lora_uart_init(void);

void lora_set_bw(sx127x_t sx127x,uint8_t lora_bw);
void lora_set_sf(sx127x_t sx127x,uint8_t lora_sf);
void lora_set_cr(sx127x_t sx127x, uint8_t cr);
void lora_set_init(void);
int lora_listen_message(void);
bool is_rssi_free(int16_t rssi_threshold);
void lora_set_channel(sx127x_t sx127x,int channel);
kernel_pid_t lora_recv_service_init(void);

/*
 * @see send message
 *
 * @param NULL
 *
 * @return RECV_ACK_TIMEOUT when send has completed, no ack data received
 * @return RECV_ACK_ERR when send has completed and error ack is received
 * @return RECV_ACK_SUCCESS when send has completed and ack is received
 * @return UNEXPECTED_VALUE when send has completed and unexpected msg type is received
*/
int lora_send_message(uint8_t *message,uint8_t message_len);

#endif
