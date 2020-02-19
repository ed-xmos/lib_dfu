// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
#ifndef __dfu_types_h__
#define __dfu_types_h__

#define DFU_BLOCK_SIZE_MAX_BYTES 512
#define DFU_MAX_PAGE_SIZE_BYTES 256
#define DFU_BLOCK_NUM_DATA_IMAGE_MARKER 0x8000

enum dfu_state {
  APP_IDLE,
  APP_DETACH,
  DFU_IDLE,
  DFU_DNLOAD_SYNC,
  DFU_DNBUSY,
  DFU_DNLOAD_IDLE,
  DFU_MANIFEST_SYNC,
  DFU_MANIFEST,
  DFU_MANIFEST_WAIT_RESET,
  DFU_UPLOAD_IDLE,
  DFU_ERROR
};

enum dfu_status {
  DFU_OK,
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

struct dfu_getstatus {
  enum dfu_status status;
  enum dfu_state state;
  unsigned poll_timeout_msec;
};

struct dfu_slots {
  unsigned boot_address;
  unsigned data_address;
};

#endif
