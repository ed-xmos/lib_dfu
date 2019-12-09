// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef __dfu_h__
#define __dfu_h__

#include <xccompat.h>
#include <quadflash.h>

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

enum dfu_status {
  DFU_OK,
  ERR_USBR,
  ERR_UNKNOWN,
  ERR_STALLED_PKT
};

enum dfu_state dfu_getstate(void);

enum dfu_status dfu_getstatus(void);

void dfu_clrstatus(void);

void dfu_detach(void);

#ifdef __XC__
void dfu_bus_reset(fl_QSPIPorts &ports, const fl_QuadDeviceSpec spec[1]);
#else
void dfu_bus_reset(fl_QSPIPorts *ports, const fl_QuadDeviceSpec spec[1]);
#endif

void dfu_timeout_detach(void);

#endif
