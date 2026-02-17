// Copyright 2017-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "dfu_commands.h"

#include <stddef.h>
#include <string.h>

#define DEBUG_UNIT DFU_COMMANDS
#define DEBUG_PRINT_ENABLE_DFU_COMMANDS 0
#include "debug_print.h"

#include "dfu.h"
#include "dfu_types.h"

/* TODO - move to conf header */
/* DFU functional descriptor wDetachTimeOut field (milliseconds)
 * Time for device to wait for bus reset after DETACH request before reverting to idle state */
#ifndef DFU_DETACH_TIME_OUT_MS
#define DFU_DETACH_TIME_OUT_MS 250
#endif


struct dfu_cmd_response dfu_handle_write_command(int32_t cmd, int32_t value, const uint8_t payload[], size_t payload_len)
{
  struct dfu_cmd_response response = { DFU_API_BAD_PARAM, 0 };
  switch (cmd) {
    case DFU_DETACH:
      dfu_detach();
      // state.timeout.enable = 1;
      // state.timeout.delta = DFU_DETACH_TIME_OUT_MS * XS1_TIMER_KHZ; // milliseconds to timer ticks
      break;

    case XMOS_BUS_RESET:
      if (dfu_getstate() == STATE_APP_DETACH) {
        // state.timeout.enable = false;
        dfu_bus_reset();
      }
      else {
        debug_printf("Unexpected bus reset DFU request\n");
        response.status = DFU_API_ERROR;
        return response;
      }
      break;

    case DFU_DNLOAD:
      dfu_dnload(value, payload_len, payload);
      break;

    case DFU_CLRSTATUS:
      dfu_clrstatus();
      break;

    case XMOS_REBOOT:
      response.value = 1;
      break;

    default:
      debug_printf("Unrecognised write command: %d\n", cmd);
      response.status = DFU_API_ERROR;
      return response;
  }
  response.status = DFU_API_SUCCESS;
  return response;
}

struct dfu_cmd_response dfu_handle_read_command(int32_t cmd, uint8_t payload[], size_t payload_len)
{
  (void)payload_len; // TODO - check length
  struct dfu_cmd_response response = { DFU_API_BAD_PARAM, 0 };

  switch (cmd) {
    case DFU_GETSTATE:
      enum dfu_state state = dfu_getstate();
      memcpy(payload, &state, sizeof(enum dfu_state));
      break;

    case DFU_GETSTATUS:
      struct dfu_getstatus ret = dfu_getstatus();
      memcpy(payload, &ret, sizeof(struct dfu_getstatus));
      break;

    case XMOS_GET_ERROR_INFO:
      int error_info = dfu_get_error_info();
      memcpy(payload, &error_info, sizeof(int));
      break;

    case DFU_UPLOAD:
      response.value = dfu_upload(payload_len, payload);
      break;

    default:
      debug_printf("Unrecognised read command: %d\n", cmd);
      response.status = DFU_API_ERROR;
      return response;
  }
  response.status = DFU_API_SUCCESS;
  return response;
}
