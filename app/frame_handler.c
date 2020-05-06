#include "frame_handler.h"
#include "frame_common.h"
#include "frame_encode.h"
// #include "log.h"
#include "periph/rtc.h"
#include "dev_cfg.h"
#include "lora_io.h"

#include <stdio.h>
#include "xtimer.h"

static uint8_t send_buf[256] = {0};
void collection_node_req_handler(node_info_req_t msg)
{
    uint32_t timestamp = msg.timestamp;
    uint32_t node_id = msg.node_id;
    float temperature = msg.temperature;
    int error_code = msg.error_code;

    printf("----->[receive] decode timestamp is %ld, node_id is %ld, temperature is %.2f, error_code is %d\r\n",
           timestamp, node_id, temperature, error_code);

    node_info_rsp_t rsp_msg = node_info_rsp_t_init_default;
    rsp_msg.node_id = node_id;
    rsp_msg.timestamp = rtc_get_counter();

    xtimer_usleep(500 * 1000);
    memset(send_buf, 0, sizeof(send_buf));
    uint16_t len = frame_temperature_rsp_data_encode(send_buf, node_info_rsp_t_fields, &rsp_msg);
    lora_io_send(send_buf, len);

    printf("<-----[send] my node id is %ld timestamp is %ld\r\n", rsp_msg.node_id, (uint32_t)rsp_msg.timestamp);
}

void collection_node_rsp_handler(node_info_rsp_t msg)
{
    uint32_t timestamp = msg.timestamp;
    uint32_t node_id = msg.node_id;

    // frame_temperature_rsp_data_encode(uint8_t * data, const pb_msgdesc_t *messagetype, node_info_rsp_t msg)

    printf("---->receive node id is %ld timestamp is %ld\r\n", node_id, timestamp);
}
