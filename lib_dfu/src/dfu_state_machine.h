// Copyright 2017-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef DFU_STATE_MACHINE_H
#define DFU_STATE_MACHINE_H

#include <stddef.h>
#include <stdint.h>

#include "dfu.h"
#include "dfu_types.h"

struct dfu_cmd_response request_with_arguments(enum dfu_request request,
                                              const uint8_t (&?write_block)[DFU_TRANSFER_SIZE_BYTES],
                                              uint8_t (&?read_block)[DFU_TRANSFER_SIZE_BYTES],
                                              int32_t block_size_bytes, int32_t &?block_num);
                                              
struct dfu_cmd_response request(enum dfu_request request);

#endif
