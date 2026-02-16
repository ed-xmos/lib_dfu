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
  struct dfu_cmd_response response = { DFU_API_BAD_PARAM, 0 };
  switch (cmd) {
    case DFU_DETACH:
      response = request(DFU_DETACH);
      break;

    case XMOS_BUS_RESET:
      dfu_bus_reset();
      response.status = DFU_API_SUCCESS;
      // TODO sort out return value here, for USB
      break;

    case DFU_DNLOAD:
      if (payload == NULL && payload_len > 0) {
        break;
      } else if (payload_len > DFU_TRANSFER_SIZE_BYTES) {
        break;
      }
      response = request_with_arguments(DFU_DNLOAD, payload, NULL, payload_len, value);
      break;

    case DFU_CLRSTATUS:
      response = request(DFU_CLRSTATUS);
      break;

    case DFU_ABORT:
      response = request(DFU_ABORT);
      break;

      case XMOS_DFU_REVERTFACTORY:
        // TODO - add support for this command
        break;

    default:
      debug_printf("Unrecognised write command: %d\n", cmd);
      response.status = DFU_API_ERROR;
      break;
  }
  return response;
}

struct dfu_cmd_response dfu_handle_read_command(int32_t cmd, uint8_t payload[], size_t payload_len)
{
  struct dfu_cmd_response response = { DFU_API_BAD_PARAM, 0 };

  switch (cmd) {
    case DFU_GETSTATE:
      if (payload == NULL || payload_len != DFU_GET_STATE_PAYLOAD_SIZE_BYTES) {
        break;
      }
      enum dfu_state state = dfu_getstate();
      payload[DFU_GETSTATE_INDEX] = (uint8_t)state;
      response.status = DFU_API_SUCCESS;
      response.return_data_len = DFU_GET_STATE_PAYLOAD_SIZE_BYTES;
      break;

    case DFU_GETSTATUS:
      if (payload == NULL || payload_len != DFU_GET_STATUS_PAYLOAD_SIZE_BYTES) {
        break;
      }
      struct dfu_getstatus ret = dfu_getstatus();
      memset(payload, 0, DFU_GET_STATUS_PAYLOAD_SIZE_BYTES);
      payload[DFU_GETSTATUS_STATUS_INDEX] = ret.status;
      memcpy(&payload[DFU_GETSTATUS_POLL_TIMEOUT_INDEX], &ret.poll_timeout_msec, DFU_GETSTATUS_POLL_TIMEOUT_BYTES);
      payload[DFU_GETSTATUS_STATE_INDEX] = ret.state;
      
      response.status = DFU_API_SUCCESS;
      response.return_data_len = DFU_GET_STATUS_PAYLOAD_SIZE_BYTES;
      break;

    case DFU_UPLOAD:
      if (payload == NULL || payload_len > DFU_TRANSFER_SIZE_BYTES) {
        break;
      }
      response = request_with_arguments(DFU_UPLOAD, null, payload, payload_len, null);
      break;

    default:
      debug_printf("Unrecognised read command: %d\n", cmd);
      response.status = DFU_API_ERROR;
      break;
  }
  return response;
}
