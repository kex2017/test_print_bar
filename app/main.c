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

static const shell_command_t shell_commands[] = {
    {"printenv", "print env info", show_dev_cfg},
    { NULL, NULL, NULL }
};

#define SAMPLING_PERIOD     2
char *test_send = "-----------2";

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

int main(void)
{
    init_dev_cfg();

    // show_dev_cfg();

    ds18_vcc_a_on();//DS18B20 power
    lora_vcc_b_on();//Ra-01 power

    xtimer_sleep(1);

    // ds18b20_init();
    // float temp = 0.0;
    // while (1) {
    //     ds18b20_get_temperature(&temp);

    //     xtimer_sleep(SAMPLING_PERIOD);
    // }
    lora_io_setup(SX127X_CHAN, SX127X_CHAN, SX127X_BW, SX127X_SF, SX127X_CR, frame_receive);
    lora_io_serv_start();
    // rtc_set_counter(1588127711);
    while (1)
    {
        xtimer_sleep(1);
        // printf("time is %ld config is 0x%02x %s\r\n", rtc_get_counter(), lora_check_bw_sf_cr(), lora_check_bw_sf_cr() == 0 ? "right":"wrong");
        lora_io_send((uint8_t*)test_send, strlen(test_send));
    }


    // lora_set_init();

    // for(int i = 0; i < 20000; i++){
    //     xtimer_usleep(100*1000);
    //     printf("is free :%d\r\n", is_rssi_free(-100));
    //     lora_send_message((uint8_t*)test_send, strlen(test_send));
    // }




    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
