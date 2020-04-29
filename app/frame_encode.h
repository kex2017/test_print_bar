#ifndef FRAME_ENCODE_H
#define FRAME_ENCODE_H

#include "proto_compiled/sctm.pb.h"

uint16_t frame_temperature_data_encode(uint8_t *data, node_info_req_t msg);

#endif
