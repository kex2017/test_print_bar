#ifndef FRAME_ENCODE_H
#define FRAME_ENCODE_H

#include "proto_compiled/sctm.pb.h"

uint16_t frame_temperature_req_data_encode(uint8_t *data, const pb_msgdesc_t *messagetype, node_info_req_t *msg);

uint16_t frame_temperature_rsp_data_encode(uint8_t *data, const pb_msgdesc_t *messagetype, node_info_rsp_t *msg);

#endif
