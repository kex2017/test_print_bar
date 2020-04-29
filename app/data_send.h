#ifndef _DATA_SEND_H
#define _DATA_SEND_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void encode_and_send_temp_data(uint32_t timestamps, float temperature, int error_code);

void frame_hander_init(void);

int frame_receive_handler(uint8_t *data, uint16_t len);

#endif
