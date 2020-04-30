#include "data_transfer.h"

#include "lora_io.h"
#include "dev_cfg.h"
#include "frame_encode.h"
#include "frame_decode.h"
#include "frame_common.h"

#define ENABLE_DEBUG (1)
#include "debug.h"
#include "frame_paser.h"
#include "xtimer.h"
#include "msg.h"

#define MAX_SEND_BUF_SIZE (256)
static uint8_t send_buf[MAX_SEND_BUF_SIZE] = {0};
#define MAX_RESEND_TIMES (5)
#define MAX_WAIT_MSG_100ms_TIMES (50)
void encode_and_send_temp_data(uint32_t timestamps, float temperature, int error_code)
{
    uint16_t len = 0;
    node_info_req_t msg = node_info_req_t_init_default;

    msg.node_id = get_dev_id();
    msg.timestamp = timestamps;
    msg.temperature = temperature;
    msg.error_code = error_code;

    len = frame_temperature_data_encode(send_buf, msg);
    lora_io_send(send_buf, len);

    msg_t recv_msg;
    for (uint8_t resend_times = 0; resend_times < MAX_RESEND_TIMES; resend_times++)
    {
        uint8_t retry_times = 0;
        lora_io_send(send_buf, len);
        while ((msg_try_receive(&recv_msg) < 0) && retry_times < MAX_WAIT_MSG_100ms_TIMES) //wait for max time :5s
        {
            xtimer_usleep(100 * 1000);
            retry_times++;
        }
        if(retry_times < MAX_WAIT_MSG_100ms_TIMES){
            printf("receive rsp msg success!!!\r\n");
            break;
        }
        else
        {
            printf("wait rsp msg over time, try resend times:%d\r\n", resend_times);
        }

    }

    // printf("encode buf as:\r\n");
    // for (uint16_t i = 0; i < len; i++)
    // {
    //     printf("%02x ", send_buf[i]);
    // }
    // printf("\r\n");
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
    int parser_frame_len = 0;

    if (!is_frame_parser_busy(&fp_dev))
    {
        add_frame_data_stream(&fp_dev, data, len);
    }
    else
    {
        DEBUG("[data recv]:frame parser busy!\r\n");
    }
    if ((parser_frame_len = do_frame_parser(&fp_dev, parser_buff, FRAME_PARSER_DATA_LEN)) > 0)
    {
        printf("parser data len :%d is :\r\n", parser_frame_len);
        for (int i = 0; i < parser_frame_len; i++)
        {
            printf("%02x ", parser_buff[i]);
        }

        DEBUG("[data recv]:frame parser ok!\r\n");
        frame_decode(parser_buff + 4, parser_frame_len - 3);
        memset(parser_buff, 0, parser_frame_len);
    }

    return 0;
}
