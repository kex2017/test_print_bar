#include "frame_handler.h"
#include "frame_common.h"
#include "frame_encode.h"
// #include "log.h"
#include "periph/rtc.h"

void collection_node_rsp_handler(node_info_rsp_t msg)
{
    uint32_t timestamp = msg.timestamp;
    uint32_t node_id = msg.node_id;

    printf("node id is %ld timestamp is %ld\r\n", node_id, timestamp);
}

