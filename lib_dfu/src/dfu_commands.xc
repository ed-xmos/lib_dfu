// Copyright 2017-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stddef.h>
#include <string.h>

#define DEBUG_UNIT DFU_COMMANDS
#define DEBUG_PRINT_ENABLE_DFU_COMMANDS 0
#include "debug_print.h"

#include "dfu.h"
#include "dfu_types.h"

// TODO - deprecated - used in tests

struct dfu_cmd_response dfu_handle_write_command(int32_t cmd, int32_t value, const uint8_t payload[], size_t payload_len)
{
  return dfu_request_with_arguments(cmd, (payload, uint8_t []), payload_len, value);
}

struct dfu_cmd_response dfu_handle_read_command(int32_t cmd, uint8_t payload[], size_t payload_len)
{
  return dfu_request_with_arguments(cmd, payload, payload_len, null);
}
