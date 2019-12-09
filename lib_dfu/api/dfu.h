// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef __dfu_h__
#define __dfu_h__

enum dfu_state {
  APP_IDLE,
  APP_DETACH,
  DFU_IDLE,
  DFU_DNLOAD_SYNC,
  DFU_DNLOAD_BUSY,
  DFU_DNLOAD_IDLE,
  DFU_MANIFEST_SYNC,
  DFU_MANIFEST,
  DFU_MANIFEST_WAIT_RESET,
  DFU_UPLOAD_IDLE,
  DFU_ERROR
};

enum dfu_state dfu_getstate(void);

#endif
