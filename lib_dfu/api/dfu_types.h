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

  XMOS_DFU_RESETDEVICE = 0xf0,  // TODO - support these
  XMOS_DFU_REVERTFACTORY = 0xf1,
  XMOS_DFU_RESETINTODFU = 0xf2,
  XMOS_DFU_RESETFROMDFU = 0xf3,
  XMOS_DFU_SELECTIMAGE = 0xf4
};

/**
 * DFU interface state machine
 */
enum dfu_state {
  STATE_APP_IDLE,
  STATE_APP_DETACH,
  STATE_DFU_IDLE,
  STATE_DFU_DOWNLOAD_SYNC,
  STATE_DFU_DOWNLOAD_BUSY,
  STATE_DFU_DOWNLOAD_IDLE,
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
  DFU_OK,
  DFU_errTARGET,
  DFU_errFILE,
  DFU_errWRITE,
  DFU_errERASE,
  DFU_errCHECK_ERASED,
  DFU_errPROG,
  DFU_errVERIFY,
  DFU_errADDRESS,
  DFU_errNOTDONE,
  DFU_errFIRMWARE,
  DFU_errVENDOR,
  DFU_errUSBR,
  DFU_errPOR,
  DFU_errUNKNOWN,
  DFU_errSTALLED_PKT
};

/**
 * Return value of GETSTATUS request
 */
struct dfu_getstatus {
  enum dfu_status status; /**< DFU Status code */
  enum dfu_state state; /**< DFU Current state */
  unsigned poll_timeout_msec; /**< Poll timeout in milliseconds */
};

/* TODO - lib_xua types, remove in time */
#define _DFU_TRANSFER_SIZE_BYTES (64)   // bMaxPacketSize0 in DFU device descriptor
#define _DFU_TRANSFER_SIZE_WORDS (_DFU_TRANSFER_SIZE_BYTES/4)
#define _FLASH_PAGE_SIZE_BYTES    (256)
#define _NUM_DFU_PAGES_PER_FLASH_PAGE (_FLASH_PAGE_SIZE_BYTES/_DFU_TRANSFER_SIZE_BYTES)

#if (_FLASH_PAGE_SIZE_BYTES % _DFU_TRANSFER_SIZE_BYTES)
#error _FLASH_PAGE_SIZE_BYTES should be a multiple of _DFU_TRANSFER_SIZE_BYTES
#endif

#endif
