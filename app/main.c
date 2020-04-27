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

uint8_t id[CPUID_LEN];
char *test_send = "-----------2";
int main(void)
{
    /* read the CPUID */
    cpuid_get(id);
     /* print the CPUID */
    printf("CPUID:");
    for (unsigned int i = 0; i < CPUID_LEN; i++) {
        printf(" 0x%02x", id[i]);
    }
    printf("\n");

    vcc_b_on();
    xtimer_sleep(1);

    lora_set_init();

    for(int i = 0; i < 200; i++){
        xtimer_sleep(5);
        lora_send_message((uint8_t*)test_send, strlen(test_send));
    }



    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
