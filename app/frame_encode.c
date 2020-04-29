#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "frame_common.h"

#include <pb_encode.h>
#include <pb_common.h>
#include "proto_compiled/sctm.pb.h"

static uint8_t proto_buffer[128];

bool encode_unionmessage(pb_ostream_t *stream, const pb_msgdesc_t *messagetype, void *message)
{
    pb_field_iter_t iter;

    if (!pb_field_iter_begin(&iter, Transfer_fields, message))
        return false;

    do
    {
        if (iter.submsg_desc == messagetype)
        {
            /* This is our field, encode the message using it. */
            if (!pb_encode_tag_for_field(stream, &iter))
                return false;

            return pb_encode_submessage(stream, messagetype, message);
        }
    } while (pb_field_iter_next(&iter));

    /* Didn't find the field for messagetype */
    return false;
}

uint16_t frame_uint16_encode(uint8_t *data, uint16_t value)
{
    uint8_t *p = (uint8_t *)&value;

    data[0] = p[1];
    data[1] = p[0];

    return sizeof(unsigned short);
}

uint16_t frame_header_encode(uint8_t *data, uint16_t rsp_len)
{
    uint16_t index = 0;

    index += frame_uint16_encode(data + index, FRAME_STARTER);
    index += frame_uint16_encode(data + index, rsp_len);

    return FRAME_HEADER_LEN;
}

uint16_t frame_protocbuf_encode(uint8_t *data, uint8_t *protoc_buf, uint16_t protoc_buf_len)
{
    uint16_t i = 0;

    for (i = 0; i < protoc_buf_len; i++)
    {
        *data++ = *protoc_buf++;
    }

    return protoc_buf_len;
}

uint16_t frame_cs_encode(uint8_t *data, uint8_t cs)
{
    return frame_uint16_encode(data, cs);
}

uint16_t frame_encode(uint8_t *data, uint8_t *pb_buf, uint32_t pb_buf_len)
{
    uint16_t index = 0;

    index += frame_header_encode(data, pb_buf_len);

    index += frame_protocbuf_encode(data + index, pb_buf, pb_buf_len);

    index += frame_cs_encode(data + index, byte_sum_checksum(data, index));

    return index;
}

uint16_t temperature_data_pb_encode(uint8_t *buffer, uint32_t buf_len, node_info_req_t msg)
{
    bool status = false;
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, buf_len);

    status = encode_unionmessage(&stream, node_info_req_t_fields, &msg);
    if (!status)
    {
        printf("Encoding set valve confirm failed!\n");
        return 0;
    }

    return stream.bytes_written;
}

/*********************************frame encode*********************************/
uint16_t frame_temperature_data_encode(uint8_t *data, node_info_req_t msg)
{
    memset(proto_buffer, 0, sizeof(proto_buffer));

    uint16_t protoc_buf_len = temperature_data_pb_encode(proto_buffer, sizeof(proto_buffer), msg);

    return frame_encode(data, proto_buffer, protoc_buf_len);
}


