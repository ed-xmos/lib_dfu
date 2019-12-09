// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <quadflash.h>
#include "dfu.h"

#define DEBUG_UNIT DFU
#define DEBUG_PRINT_ENABLE_DFU 0
#include "debug_print.h"

static enum dfu_state state = APP_IDLE;
static enum dfu_status status = DFU_OK;

#if DEBUG_PRINT_ENABLE_DFU
static const char * unsafe state_str(enum dfu_state s)
{
  unsafe {
    switch (s) {
      case APP_IDLE:                  return "appIDLE";
      case APP_DETACH:                return "appDETACH";
      case DFU_IDLE:                  return "dfuIDLE";
      case DFU_DNLOAD_SYNC:           return "dfuDNLOAD-SYNC";
      case DFU_DNLOAD_BUSY:           return "dfuDNLOAD-BUSY";
      case DFU_DNLOAD_IDLE:           return "dfuDNLOAD-IDLE";
      case DFU_MANIFEST_SYNC:         return "dfuMANIFEST-SYNC";
      case DFU_MANIFEST:              return "dfuMANIFEST";
      case DFU_MANIFEST_WAIT_RESET:   return "dfuMANIFEST-WAIT-RESET";
      case DFU_UPLOAD_IDLE:           return "dfuUPLOAD-IDLE";
      case DFU_ERROR:                 return "dfuERROR";
      default:                        return "?";
    }
  }
}
#endif

static void transition(enum dfu_state new)
{
  unsafe {
    debug_printf("DFU state: %s -> %s\n", state_str(state), state_str(new));
  }
  state = new;
}

enum dfu_state dfu_getstate(void)
{
  return state;
}

enum dfu_status dfu_getstatus(void)
{
  return status;
}

void dfu_clrstatus(void)
{
  status = DFU_OK;
  transition(DFU_IDLE);
}

void dfu_detach(void)
{
  if (state == APP_IDLE) {
    transition(APP_DETACH);
  }
  else if (state == APP_DETACH) {
    status = ERR_STALLED_PKT;
    transition(APP_IDLE);
  }
  else {
    status = ERR_STALLED_PKT;
    transition(DFU_ERROR);
  }
}

void dfu_bus_reset(fl_QSPIPorts &ports, const fl_QuadDeviceSpec spec[1])
{
  if (state == APP_DETACH) {
    int ret = fl_connectToDevice(ports, spec, 1);
    if (ret == 0) {
      transition(DFU_IDLE);
    }
    else {
      debug_printf("error: quadflash connectToDevice returned %d\n", ret);
      status = ERR_UNKNOWN;
      transition(DFU_ERROR);
    }
  }
  else if (state == APP_IDLE) {
    status = ERR_USBR;
  }
  else {
    status = ERR_USBR;
    transition(DFU_ERROR);
  }
}

void dfu_timeout_detach(void)
{
  if (state == APP_DETACH) {
    transition(APP_IDLE);
  }
  else {
    debug_printf("unexpected detach timeout\n");
  }
}
