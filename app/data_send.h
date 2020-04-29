#ifndef _DATA_SEND_H
#define _DATA_SEND_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void encode_and_send_temp_data(uint32_t timestamps, float temperature, int error_code);

#endif
