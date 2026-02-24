// Copyright 2019-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef DFU_H
#define DFU_H

#include <stddef.h>
#include <quadflash.h>
#include <stdint.h>

#include "dfu_default_conf.h"
#include "dfu_types.h"

/** API function return values */
enum dfu_api_status {
  DFU_API_SUCCESS = 0,
  DFU_API_ERROR = 1,
  DFU_API_DATA_LENGTH_ERROR = 2,
  DFU_API_BAD_PARAM = 3
};

enum dfu_reset_type
{
    DFU_RESET_TYPE_NONE,
    DFU_RESET_TYPE_RESET_TO_DFU,
    DFU_RESET_TYPE_RESET_TO_APP
};

struct dfu_cmd_response {
  enum dfu_api_status status;
  int32_t return_data_len;
  enum dfu_reset_type reset_type;
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

/**
 * \defgroup lib_dfu_api API
 * \{
 */

/**
 * DFU DETACH request
 */
void dfu_detach(void);

/**
 * Simulate a USB bus reset
 *
 * Normal implementation based on USB DFU specification would undergo an actual
 * bus reset. In our implementation we want to not add any code for handling
 * USB reset into DFU, and we also want to support DFU over I2C with this code.
 * Therefore we call a function to advance the state machine. It does nothing
 * else than change the interface state.
 */
void dfu_bus_reset(void);

/**
 * Tell the state machine that detach timeout occurred
 *
 * The usage scheme is for the caller to implement the timeout based on value
 * exposed in USB descriptors. Adding timers in the library doesn't seem very
 * useful, it's best for the caller to integrate.
 *
 * In non-USB applications (such as I2S/I2C) we leave this timeout facility
 * unused (never call this function). In USB applications it is largely there
 * for specification compliance (like the bus reset).
 */
void dfu_timeout_detach(void);

/**
 * DFU DNLOAD request
 *
 * Block size can vary, but normally doesn't. Typical use is a sequence of fixed
 * size blocks until the end of an image, then one zero-size block to finish.
 *
 * Note that at this point the caller must have connected to the flash using
 * quadflash library. While DNLOAD request does no erasing or writing work, it
 * needs to know the page size to being converting blocks to pages.
 *
 * \param block_num          Block number
 * \param block_size_bytes   Block size in bytes
 * \param block              Block contents
 */
void dfu_dnload(int32_t block_num, int32_t block_size_bytes,
                const uint8_t block[DFU_TRANSFER_SIZE_BYTES]);

/**
 * DFU  UPLOAD request
 *
 * Block size can vary, but normally doesn't. Typical use is a sequence of fixed
 * size blocks until the end of an image, then one zero-size block to finish.
 *
 * Note that at this point the caller must have connected to the flash using
 * quadflash library. While UPLOAD request does no erasing or writing work, it
 * needs to know the page size to being converting blocks to pages.
 *
 * \param block_size_bytes   Block size in bytes
 * \param block              Block contents
 * 
 * \return Block number of the block returned in the block parameter. This is useful for the caller to track the progress of the upload.
 */
int32_t dfu_upload(int32_t block_size_bytes, uint8_t read_block[DFU_TRANSFER_SIZE_BYTES]);

/**
 * DFU CLRSTATUS request
 */
void dfu_clrstatus(void);

/** \} */

#endif
