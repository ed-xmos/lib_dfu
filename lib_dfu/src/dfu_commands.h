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

#define RESOURCE_ID_DFU 0xD0

// Timeout redundant with "bitWillDetach" in DFU functional descriptor
// struct dfu_timeout {
//   int32_t enable;
//   unsigned delta;
// };

/** API function return values */
enum dfu_api_status {
  DFU_API_SUCCESS = 0,
  DFU_API_ERROR = 1,
  DFU_API_DATA_LENGTH_ERROR = 2,
  DFU_API_BAD_PARAM = 3
};

struct dfu_cmd_response {
  enum dfu_api_status status;
  int32_t value;
};

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

 /** DFU host write request handling
  *
  * \param cmd - the DFU command (bRequest)
  * \param value - the wValue field of the request, usage depends on command, either block-num for download or timeout for detach
  * \param payload - pointer to the data payload of the request, usage depends on command
  * \param payload_len - length of the data payload in bytes
  * 
  * \return struct dfu_cmd_response containing status and any return value
  * \retval DFU_API_SUCCESS if command was handled successfully, the value will mark whether device needs a reboot
  * \retval DFU_API_ERROR if there was an error handling the command
  * \retval DFU_API_BAD_PARAM if the command or parameters were invalid
  */
struct dfu_cmd_response dfu_handle_write_command(int32_t cmd, int32_t value, const uint8_t payload[], size_t payload_len);

/** DFU host read request handling
 * 
 * \param cmd - the DFU command (bRequest)
 * \param payload - pointer to the data payload buffer to be filled by the command handler, usage depends on command
 * \param payload_len - length of the data payload buffer in bytes
 * 
 * \return struct dfu_cmd_response containing status and any return value
 * \retval DFU_API_SUCCESS if command was handled successfully, the value is the upload block-number, 0 otherwise.
 * \retval DFU_API_ERROR if there was an error handling the command
 * \retval DFU_API_BAD_PARAM if the command or parameters were invalid
 */
struct dfu_cmd_response dfu_handle_read_command(int32_t cmd, uint8_t payload[], size_t payload_len);

#endif
