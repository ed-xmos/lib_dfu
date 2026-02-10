// Copyright 2017-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef DFU_COMMANDS_H
#define DFU_COMMANDS_H

#ifdef __xcore__
#include <platform.h>
#include <quadflash.h>
#endif
#include <stddef.h>
#include <stdint.h>
#ifdef __XC__
#define _Bool int
#endif
#include <stdbool.h>

#define RESOURCE_ID_DFU 0xD0

// Timeout redundant with "bitWillDetach" in DFU functional descriptor
// struct dfu_timeout {
//   bool enable;
//   unsigned delta;
// };

struct dfu_write_command_state {
  // struct dfu_timeout timeout;
  bool needs_reboot;
};

#ifdef __XC__

/* From USB DFU spec v1.1 
 *
 * bRequest,      wValue,   wIndex,   wLength,  Data
 * -------------------------------------------------
 * DFU_DETACH     wTimeout  Interface Zero      None
 * DFU-DNLOAD     wBlockNum Interface wLength   Block
 * DFU_UPLOAD     wBlockNum Interface wLength   Block
 * DFU_GETSTATUS  Zero      Interface 6         Status
 * DFU_CLRSTATUS  Zero      Interface Zero      None
 * DFU_GETSTATE   Zero      Interface 1         State
 * DFU_ABORT      Zero      Interface Zero      None
 */

int32_t dfu_handle_write_command(int32_t cmd, int32_t value, const uint8_t payload[],
                                 size_t payload_len,
                                 struct dfu_write_command_state &state);

int32_t dfu_handle_read_command(int32_t cmd, int32_t &value, uint8_t payload[], size_t payload_len);
#endif

#endif
