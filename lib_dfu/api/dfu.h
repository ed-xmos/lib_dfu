// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef __dfu_h__
#define __dfu_h__

#include <stddef.h>
#include <xccompat.h>
#include <quadflash.h>

#define DFU_BLOCK_SIZE_MAX_BYTES 32

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
  ERR_USBR,
  ERR_UNKNOWN,
  ERR_STALLED_PKT
};

enum dfu_state dfu_getstate(void);

#ifdef __XC__
{enum dfu_status, enum dfu_state, unsigned} dfu_getstatus(void);
#endif

void dfu_clrstatus(void);

void dfu_detach(void);

void dfu_bus_reset(REFERENCE_PARAM(fl_QSPIPorts, ports),
                   const fl_QuadDeviceSpec spec[1]);

void dfu_timeout_detach(void);

void dfu_dnload(unsigned short block_num, size_t block_size_bytes,
                const char block_data[DFU_BLOCK_SIZE_MAX_BYTES]);

#endif
