// Copyright 2019-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef DFU_TYPES_H
#define DFU_TYPES_H

/**
 * Divide 16bit block number space in half. Top bit cleared is for boot
 * partition. Top bit set (the below marker value) is for data partition.
 *
 * Note that this creates a gap in the sequence of block numbers presented
 * while sending a boot image followed by a data image. For instance, it the
 * sequence goes 0 up to 8,000 for a boot image. Then jump to 32,768 and
 * continue incrementing to 40,000 for the data image.
 *
 * Currently not intended as actual specification-compliant deployment, in
 * the future we will need to check the gap does not upset some driver
 * software or third party utilities.
 */
#define DFU_BLOCK_NUM_DATA_IMAGE_MARKER 0x8000

/**
 * DFU request types
 */
enum dfu_request {
  // USB spec DFU commands
  DFU_DETACH = 0,
  DFU_DNLOAD = 1,
  DFU_UPLOAD = 2,     // TODO - add support
  DFU_GETSTATUS = 3,
  DFU_CLRSTATUS = 4,
  DFU_GETSTATE = 5,
  DFU_ABORT = 6,      // TODO - add support
  // XMOS custom DFU commands - values chosen to avoid conflict with standard DFU requests
  XMOS_REBOOT = 7,          // For host requesting device reboot
  XMOS_GET_ERROR_INFO = 8,  // TODO - check the usage of this
  XMOS_BUS_RESET = 9,       // For simulating bus/device reset on transports other than USB.
};

/**
 * DFU interface state machine
 */
enum dfu_state {
  STATE_APP_IDLE,
  STATE_APP_DETACH,
  STATE_DFU_IDLE,
  STATE_DFU_DNLOAD_SYNC,
  STATE_DFU_DNBUSY,
  STATE_DFU_DNLOAD_IDLE,
  STATE_DFU_MANIFEST_SYNC,
  STATE_DFU_MANIFEST,
  STATE_DFU_MANIFEST_WAIT_RESET,
  STATE_DFU_UPLOAD_IDLE,
  STATE_DFU_ERROR
};

/**
 * DFU device status code
 */
enum dfu_status {
  ERR_OK,
  ERR_TARGET,
  ERR_FILE,
  ERR_WRITE,
  ERR_ERASE,
  ERR_CHECK_ERASED,
  ERR_PROG,
  ERR_VERIFY,
  ERR_ADDRESS,
  ERR_NOTDONE,
  ERR_FIRMWARE,
  ERR_VENDOR,
  ERR_USBR,
  ERR_POR,
  ERR_UNKNOWN,
  ERR_STALLED_PKT
};

/**
 * Return value of GETSTATUS request
 */
struct dfu_getstatus {
  enum dfu_status status; /**< DFU Status code */
  enum dfu_state state; /**< DFU Current state */
  unsigned poll_timeout_msec; /**< Poll timeout in milliseconds */
};

/** API function return values */
enum dfu_api_status {
  DFU_API_SUCCESS = 0,
  DFU_API_ERROR = 1,
  DFU_API_DATA_LENGTH_ERROR = 2,
  DFU_API_BAD_PARAM = 3
};

#endif
