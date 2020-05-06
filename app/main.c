#include <stdio.h>
#include <string.h>

#include "board.h"
#include "shell.h"
#include "stdio_uart.h"

#include "periph/cpuid.h"
#include "xtimer.h"
#include "lora_set.h"
#include "ds18b20.h"
#include "lora_io.h"
#include "periph/rtc.h"

#include "dev_cfg.h"
#include "sc_dev_cfg.h"
#include "frame_decode.h"
#include "data_transfer.h"

static const shell_command_t shell_commands[] = {
    {"setenv", "set env cfg", set_env_cfg},
    {"printenv", "print env cfg", show_dev_cfg},
    {NULL, NULL, NULL}};

#define SAMPLING_PERIOD 2

int frame_receive(uint8_t *data, uint16_t len)
{
    printf("recv data len is %d:\r\n", len);
    for (uint16_t i = 0; i < len; i++)
    {
        printf("%c ", data[i]);
    }
    printf("\r\n");

    return 0;
}

uint8_t decode_test[] = {0x55, 0xff, 0x00, 0x0c, 0x0a, 0x0a, 0x08, 0x1b, 0x10, 0xea, 0x07, 0x1d, 0xe1, 0x7a, 0xd4, 0x41, 0x00, 0x5b};

extern void xtimer_frame_init(uint32_t char_interval);
extern void wait_print_frame(void);
int main(void)
{
    xtimer_frame_init(1 * 1000);

    while (1)
    {
        xtimer_usleep(100 * 1000);
        wait_print_frame();
    }


    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
