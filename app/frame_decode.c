#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pb_decode.h>
#include <pb_common.h>
#include "proto_compiled/sctm.pb.h"
#include "msg.h"

#include "frame_handler.h"

const pb_msgdesc_t *decode_unionmessage_type(pb_istream_t *stream)
{
    pb_wire_type_t wire_type;
    uint32_t tag;
    bool eof;

    while (pb_decode_tag(stream, &wire_type, &tag, &eof))
    {
        if (wire_type == PB_WT_STRING)
        {
            pb_field_iter_t iter;
            if (pb_field_iter_begin(&iter, Transfer_fields, NULL) &&
                pb_field_iter_find(&iter, tag))
            {
                /* Found our field. */
                return iter.submsg_desc;
            }
        }

        /* Wasn't our field.. */
        pb_skip_field(stream, wire_type);
    }

    return NULL;
}

bool decode_unionmessage_contents(pb_istream_t *stream, const pb_msgdesc_t *messagetype, void *dest_struct)
{
    pb_istream_t substream;
    bool status;
    if (!pb_make_string_substream(stream, &substream))
        return false;

    status = pb_decode(&substream, messagetype, dest_struct);
    pb_close_string_substream(stream, &substream);
    return status;
}

kernel_pid_t sample_pid;

void get_sample_pid_hook(kernel_pid_t pid)
{
    sample_pid = pid;
}

int frame_decode(uint8_t *buffer, uint16_t count)
{
    /* Read the data into buffer */
    pb_istream_t stream = pb_istream_from_buffer(buffer, count);

    const pb_msgdesc_t *type = decode_unionmessage_type(&stream);
    bool status = false;

    if (type == node_info_req_t_fields)
    {
        node_info_req_t msg = node_info_req_t_init_default;
        status = decode_unionmessage_contents(&stream, node_info_req_t_fields, &msg);
        // printf("-------------->[receive]: node %ld temperature %.2f\r\n", msg.node_id, msg.temperature);
        collection_node_req_handler(msg);
    }
    else if (type == node_info_rsp_t_fields)
    {
        node_info_rsp_t msg = {};
        status = decode_unionmessage_contents(&stream, node_info_rsp_t_fields, &msg);
        printf("-------------->[receive]: from gateway node_info_rsp handler---------------\r\n");
        // collection_node_rsp_handler(msg);

        // msg_t send_msg;
        // msg_send(&send_msg, sample_pid);
        // printf("receive rsp send msg pid is %d\r\n", sample_pid);
    }
    else
    {
        printf("invalid type !\r\n");
    }

    if (!status)
    {
        printf("Decode failed: %s\n", PB_GET_ERROR(&stream));
        return 1;
    }

    return 0;
}
