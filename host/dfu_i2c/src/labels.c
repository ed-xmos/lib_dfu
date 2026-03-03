// Copyright 2020-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "labels.h"

const char *command_str(int command)
{
  switch (command) {
    case DFU_DETACH:            return "DETACH";
    case XMOS_BUS_RESET:        return "BUS_RESET";
    case DFU_DNLOAD:            return "DNLOAD";
    case DFU_CLRSTATUS:         return "CLRSTATUS";
    case DFU_GETSTATE:          return "GETSTATE";
    case DFU_GETSTATUS:         return "GETSTATUS";
    case XMOS_DFU_REVERTFACTORY:return "REVERT_FACTORY";
    default: return "?";
  }
}

const char *state_str(enum dfu_state state)
{
  switch (state) {
    case STATE_APP_IDLE:                return "appIDLE";
    case STATE_APP_DETACH:              return "appDETACH";
    case STATE_DFU_IDLE:                return "dfuIDLE";
    case STATE_DFU_DOWNLOAD_SYNC:       return "dfuDNLOAD-SYNC";
    case STATE_DFU_DOWNLOAD_BUSY:       return "dfuDNBUSY";
    case STATE_DFU_DOWNLOAD_IDLE:       return "dfuDNLOAD-IDLE";
    case STATE_DFU_MANIFEST_SYNC:       return "dfuMANIFEST-SYNC";
    case STATE_DFU_MANIFEST:            return "dfuMANIFEST";
    case STATE_DFU_MANIFEST_WAIT_RESET: return "dfuMANIFEST-WAIT-RESET";
    case STATE_DFU_UPLOAD_IDLE:         return "dfuUPLOAD-IDLE";
    case STATE_DFU_ERROR:               return "dfuERROR";
    default: return "?";
  }
}

const char *status_str(enum dfu_status status)
{
  switch (status) {
    case DFU_OK:              return "OK";
    case DFU_errTARGET:       return "errTARGET";
    case DFU_errFILE:         return "errFILE";
    case DFU_errWRITE:        return "errWRITE";
    case DFU_errERASE:        return "errFILE";
    case DFU_errCHECK_ERASED: return "errCHECK_ERASED";
    case DFU_errPROG:         return "errPROG";
    case DFU_errVERIFY:       return "errVERIFY";
    case DFU_errADDRESS:      return "errADDRESS";
    case DFU_errNOTDONE:      return "errNOTDONE";
    case DFU_errFIRMWARE:     return "errFIRMWARE";
    case DFU_errVENDOR:       return "errVENDOR";
    case DFU_errUSBR:         return "errUSBR";
    case DFU_errPOR:          return "errPOR";
    case DFU_errUNKNOWN:      return "errUNKNOWN";
    case DFU_errSTALLED_PKT:  return "errSTALLEDPKT";
    default: return "?";
  }
}
