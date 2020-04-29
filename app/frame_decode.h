#ifndef FRAME_DECODE_H
#define FRAME_DECODE_H

#include <stdio.h>

#include "kernel_types.h"

int frame_decode(uint8_t *buffer, uint16_t count);

void get_sample_pid_hook(kernel_pid_t pid);

#endif
