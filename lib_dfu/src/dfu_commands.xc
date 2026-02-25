// Copyright 2017-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stddef.h>
#include <string.h>

#define DEBUG_UNIT DFU_COMMANDS
#define DEBUG_PRINT_ENABLE_DFU_COMMANDS 0
#include "debug_print.h"

#include "dfu.h"
#include "dfu_types.h"
#include "dfu_state_machine.h"


struct dfu_cmd_response dfu_handle_write_command(int32_t cmd, int32_t value, const uint8_t payload[], size_t payload_len)
{
  struct dfu_cmd_response response = { DFU_API_BAD_PARAM, 0, DFU_RESET_TYPE_NONE };

  if (cmd == XMOS_BUS_RESET) {
    dfu_bus_reset();
    response.status = DFU_API_SUCCESS;

  } else if (cmd == XMOS_DFU_REVERTFACTORY) {
    // TODO - add support for this command
    // response.status = DFU_API_SUCCESS;

  } else {
    response = request_with_arguments(cmd, payload, null, payload_len, value);
  }
  return response;
}

struct dfu_cmd_response dfu_handle_read_command(int32_t cmd, uint8_t payload[], size_t payload_len)
{
  return request_with_arguments(cmd, null, payload, payload_len, null);
}
