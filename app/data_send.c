#include "data_send.h"
#include "lora_io.h"
#include "dev_cfg.h"

#define MAX_SEND_BUF_SIZE (256)
static uint8_t send_buf[MAX_SEND_BUF_SIZE] = {0};

void encode_and_send_temp_data(uint32_t timestamps, float temperature, int error_code)
{
    uint16_t len = 0;
    // uint16_t len = encode_temp();
    len = sprintf((char*)send_buf, "my id is %ld timestamp is %ld temperature is %.2f, error code is %d", get_dev_id(), timestamps, temperature, error_code);
    lora_io_send(send_buf, len);
    puts((char*)send_buf);
}
