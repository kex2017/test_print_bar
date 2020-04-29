#ifndef LORA_IO_H
#define LORA_IO_H

#include <stdint.h>
#include <stdbool.h>

#define LORA_IO_PAYLOAD_LEN 150

typedef int(*lora_io_recv_handler_t)(uint8_t *data, uint16_t len);
void lora_io_setup(int i_chan,int o_chan,uint8_t bw,uint8_t sf,uint8_t cr,lora_io_recv_handler_t lora_io_recv_handler);
void lora_io_serv_start(void);
void lora_io_change_channel(int i_chan,int o_chan);
int lora_io_send(uint8_t* data,uint16_t len);
bool lora_set_syncword(uint8_t syncword);
bool is_channel_free(uint32_t freq, int16_t rssi_threshold);
uint8_t lora_check_bw_sf_cr(void);
#endif
