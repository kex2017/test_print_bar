#include "data_send.h"
#include "lora_io.h"
#include "dev_cfg.h"
#include "frame_encode.h"
#include "frame_decode.h"
#include "frame_paser.h"
#include "frame_common.h"

#define MAX_SEND_BUF_SIZE (256)
static uint8_t send_buf[MAX_SEND_BUF_SIZE] = {0};

void encode_and_send_temp_data(uint32_t timestamps, float temperature, int error_code)
{
    uint16_t len = 0;
    node_info_req_t msg = node_info_req_t_init_default;

    msg.node_id = get_dev_id();
    msg.timestamp = timestamps;
    msg.temperature = temperature;
    msg.error_code = error_code;

    len = frame_temperature_data_encode(send_buf, msg);
    // len = sprintf((char*)send_buf, "my id is %ld timestamp is %ld temperature is %.2f, error code is %d", get_dev_id(), timestamps, temperature, error_code);
    lora_io_send(send_buf, len);

    printf("encode buf as:\r\n");
    for (uint16_t i = 0; i < len; i++)
    {
        printf("%02x ", send_buf[i]);
    }
    printf("\r\n");
    // puts((char*)send_buf);
}

#define FRAME_PARSER_DATA_LEN (128)
/* Frame parser data thread start */
uint8_t parser_buff[FRAME_PARSER_DATA_LEN] = {0};
char rx_buff[FRAME_PARSER_DATA_LEN] = {0};

#define HEAD_LIST_LEN (2)
uint8_t head[HEAD_LIST_LEN] = {0x55, 0xff};

#define CHECK_LEN (2)

static frame_paser_dev_t fp_dev;

int checksum(uint8_t *data, uint16_t data_len, uint8_t *crc_buf, uint16_t crc_len)
{
    (void)crc_len;
    crc_buf[1] = byte_sum_checksum(data, data_len);

    return sizeof(uint16_t);
}

void frame_hander_init(void)
{
    if (0 > frame_paser_init(&fp_dev, rx_buff, FRAME_PARSER_DATA_LEN, head, HEAD_LIST_LEN, checksum,
                             CHECK_LEN))
    {
        printf("Frame parser init error\r\n");
    }
}

int frame_receive_handler(uint8_t *data, uint16_t len)
{

    if (!is_frame_parser_busy(&fp_dev))
    {
        add_frame_data_stream(&fp_dev, data, len);
    }
    else
    {
        printf("frame parser busy!\r\n");
    }
    if ((len = do_frame_parser(&fp_dev, parser_buff, FRAME_PARSER_DATA_LEN)) > 0)
    {
        printf("do frame parser...");
        frame_decode(parser_buff + 4, len - 3);
        memset(parser_buff, 0, len);
    }

    return 0;
}
