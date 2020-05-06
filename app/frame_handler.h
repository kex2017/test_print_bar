#ifndef FRAME_HANDLER_H
#define FRAME_HANDLER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pb_decode.h>
#include <pb_common.h>
#include "proto_compiled/sctm.pb.h"

void collection_node_req_handler(node_info_req_t msg);

void collection_node_rsp_handler(node_info_rsp_t msg);

#endif
