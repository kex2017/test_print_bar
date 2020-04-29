#ifndef FRAME_H_
#define FRAME_H_

#include <stdint.h>

#define FRAME_STARTER (0x55FF)
#define FRAME_HEADER_LEN (4)

uint8_t byte_sum_checksum(uint8_t *data, uint32_t length);

#endif
