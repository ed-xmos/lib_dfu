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


int32_t dfu_handle_write_command(int32_t cmd, int32_t value, const uint8_t payload[],
                                 size_t payload_len,
                                 struct dfu_write_command_state &state)
{
  switch (cmd) {
    case DFU_DETACH:
      dfu_detach();
      // state.timeout.enable = true;
      // state.timeout.delta = DFU_DETACH_TIME_OUT_MS * XS1_TIMER_KHZ; // milliseconds to timer ticks
      break;

    case XMOS_BUS_RESET:
      if (dfu_getstate() == STATE_APP_DETACH) {
        // state.timeout.enable = false;
        dfu_bus_reset();
      }
      else {
        debug_printf("Unexpected bus reset DFU request\n");
        return DFU_API_ERROR;
      }
      break;

    case DFU_DNLOAD:
      dfu_dnload(value, payload_len, payload);
      break;

    case DFU_CLRSTATUS:
      dfu_clrstatus();
      break;

    case XMOS_REBOOT:
      state.needs_reboot = true;
      break;

    default:
      debug_printf("Unrecognised write command: %d\n", cmd);
      return DFU_API_ERROR;
  }
  return DFU_API_SUCCESS;
}

int32_t dfu_handle_read_command(int32_t cmd, int32_t &value, uint8_t payload[], size_t payload_len)
{
  (void)payload_len; // TODO - check length

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
      value = dfu_upload(payload_len, payload);
      break;

    default:
      debug_printf("Unrecognised read command: %d\n", cmd);
      return DFU_API_ERROR;
  }
  return DFU_API_SUCCESS;
}
