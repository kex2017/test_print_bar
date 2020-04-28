#include <stdio.h>
#include <string.h>

#include "board.h"
#include "shell.h"
#include "stdio_uart.h"

#include "periph/cpuid.h"
#include "xtimer.h"
#include "lora_set.h"

static const shell_command_t shell_commands[] = {
    { NULL, NULL, NULL }
};

char *test_send = "-----------2";
int main(void)
{
    vcc_b_on();
    xtimer_sleep(1);

    lora_set_init();

    for(int i = 0; i < 200; i++){
        xtimer_sleep(3);
        lora_send_message((uint8_t*)test_send, strlen(test_send));
    }

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
