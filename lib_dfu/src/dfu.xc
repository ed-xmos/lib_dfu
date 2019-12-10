// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <xs1.h>

#define DEBUG_UNIT DFU
#define DEBUG_PRINT_ENABLE_DFU 0
#include "debug_print.h"

#include "dfu_buffer_converter.h"
#include "dfu_flash.h"
#include "dfu.h"

#define POLL_TIMEOUT_MS 1

static enum dfu_state state = APP_IDLE;
static enum dfu_status status = DFU_OK;
static unsigned page_size_bytes = 0;
static fl_BootImageInfo preceding; // TODO initial state
static struct buffer_converter converter;

enum dfu_request {
  DFU_DETACH,
  DFU_DNLOAD,
  DFU_UPLOAD,
  DFU_GETSTATUS,
  DFU_CLRSTATUS,
  DFU_GETSTATE,
  DFU_ABORT
};

#if DEBUG_PRINT_ENABLE_DFU
static const char * unsafe state_str(enum dfu_state s)
{
  unsafe {
    switch (s) {
      case APP_IDLE:                  return "appIDLE";
      case APP_DETACH:                return "appDETACH";
      case DFU_IDLE:                  return "dfuIDLE";
      case DFU_DNLOAD_SYNC:           return "dfuDNLOAD-SYNC";
      case DFU_DNBUSY:                return "dfuDNBUSY";
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

static const char * unsafe status_str(enum dfu_status s)
{
  unsafe {
    switch (s) {
      case DFU_OK:                    return "OK";
      case ERR_TARGET:                return "errTARGET";
      case ERR_FILE:                  return "errFILE";
      case ERR_WRITE:                 return "errWRITE";
      case ERR_ERASE:                 return "errFILE";
      case ERR_CHECK_ERASED:          return "errCHECK_ERASED";
      case ERR_PROG:                  return "errPROG";
      case ERR_VERIFY:                return "errVERIFY";
      case ERR_ADDRESS:               return "errADDRESS";
      case ERR_NOTDONE:               return "errNOTDONE";
      case ERR_FIRMWARE:              return "errFIRMWARE";
      case ERR_VENDOR:                return "errVENDOR";
      case ERR_USBR:                  return "errUSBR";
      case ERR_POR:                   return "errPOR";
      case ERR_UNKNOWN:               return "errUNKNOWN";
      case ERR_STALLED_PKT:           return "errSTALLEDPKT";
    }
  }
}
#endif

static void normal_transition(enum dfu_state new)
{
  unsafe {
    debug_printf("DFU: %s -> %s (OK)\n", state_str(state), state_str(new));
  }
  status = DFU_OK;
  state = new;
}

static void error_condition(enum dfu_status code)
{
  unsafe {
    debug_printf("DFU: %s -> DFU_ERROR (%s)\n", state_str(state), status_str(code));
  }
  status = code;
  state = DFU_ERROR;
}

static int push_block_try_pull_page(const char block[], int block_size_bytes,
                                    char page[], int page_size_bytes)
{
  // page size is not known, might not be connected to flash
  if (page_size_bytes == 0)
    return -1;

  if (page_size_bytes > DFU_PAGE_SIZE_MAX_BYTES)
    return -2;

  // not implemented large block sizes of multiple pages
  // only small block sizes that multiply up to one page
  if (block_size_bytes > page_size_bytes)
    return -3;

  if (buffer_converter_push(converter, block, block_size_bytes) != 0)
    return -4; // not enough space - missed some pulls?

  if (buffer_converter_pull(converter, page, page_size_bytes) != 0)
    return 0; // not enough blocks pushed to make a page
  else
    return page_size_bytes; // got one whole page
}

static void request_with_arguments(enum dfu_request request,
                                   const char (&?write_block)[DFU_BLOCK_SIZE_MAX_BYTES],
                                   char (&?read_block)[DFU_BLOCK_SIZE_MAX_BYTES],
                                   int block_size_bytes)
{
  char page[DFU_PAGE_SIZE_MAX_BYTES];

  switch (state) {
    case APP_IDLE:
      if (request == DFU_DETACH) {
        normal_transition(APP_DETACH);
      }
      // no other requests expected, stay in appIDLE
      break;

    case APP_DETACH:
      if (request != DFU_GETSTATUS && request != DFU_GETSTATE) {
        // no other requests expected, return to appIDLE
        normal_transition(APP_IDLE);
      }
      break;

    case DFU_IDLE:
      if (request == DFU_DNLOAD) {
        if (flash_prepare_image_write(preceding) != 0) {
          error_condition(ERR_UNKNOWN);
          break;
        }
        int pulled = push_block_try_pull_page(write_block, block_size_bytes,
                                              page, page_size_bytes);
        if (pulled == -1) {
          error_condition(ERR_UNKNOWN);
          break;
        }

        if (pulled > 0) {
          if (flash_begin_page_write(page, page_size_bytes) != 0) {
            error_condition(ERR_UNKNOWN);
            break;
          }
        }

        normal_transition(DFU_DNLOAD_SYNC);
      }
      else if (request != DFU_GETSTATUS && request != DFU_GETSTATE) {
        // no other requests expected, defined as error
        error_condition(ERR_STALLED_PKT);
      }
      break;

    case DFU_DNLOAD_SYNC:
      if (request == DFU_GETSTATUS) {
        if (flash_has_page_write_completed()) {
          normal_transition(DFU_DNLOAD_IDLE);
        }
        else {
          normal_transition(DFU_DNBUSY);
          normal_transition(DFU_DNLOAD_SYNC);
        }
      }
      break;

    case DFU_MANIFEST_SYNC:
      if (request == DFU_GETSTATUS) {
        // completed straight away
        buffer_converter_reset(converter);
        normal_transition(DFU_IDLE);
        // not disconnecting from flash to allow additional operations
      }
      else {
        error_condition(ERR_STALLED_PKT);
      }
      break;

    case DFU_DNLOAD_IDLE:
      if (block_size_bytes == 0) {
        if (flash_finalise_image_write() != 0) {
          error_condition(ERR_UNKNOWN);
          break;
        }
        normal_transition(DFU_MANIFEST_SYNC);
      }
      else {
        int pulled = push_block_try_pull_page(write_block, block_size_bytes,
                                              page, page_size_bytes);
        if (pulled == -1) {
          error_condition(ERR_UNKNOWN);
          break;
        }

        if (pulled > 0) {
          if (flash_begin_page_write(page, page_size_bytes) != 0) {
            error_condition(ERR_UNKNOWN);
            break;
          }
        }

        normal_transition(DFU_DNLOAD_SYNC);
      }
      break;

    case DFU_ERROR:
      if (request == DFU_CLRSTATUS) {
        buffer_converter_reset(converter);
        normal_transition(DFU_IDLE);
      }
      break;
  }
}

static void request(enum dfu_request request)
{
  request_with_arguments(request, null, null, 0);
}

static void bus_reset(fl_QSPIPorts &ports, const fl_QuadDeviceSpec spec[1])
{
  if (state == APP_DETACH) {
    int ret = flash_connect(ports, spec);
    if (ret == 0) {
      page_size_bytes = spec[0].pageSize;
      buffer_converter_reset(converter);
      normal_transition(DFU_IDLE);
    }
    else {
      debug_printf("error: quadflash connectToDevice returned %d\n", ret);
      error_condition(ERR_UNKNOWN);
    }
  }
  else if (state == APP_IDLE) {
    normal_transition(APP_IDLE);
  }
  else {
    error_condition(ERR_USBR);
  }
}

static void timeout_detach(void)
{
  if (state == APP_DETACH) {
    normal_transition(APP_IDLE);
  }
  else {
    debug_printf("unexpected detach timeout call\n");
    // remain in current state, no error code indication
  }
}

enum dfu_state dfu_getstate(void)
{
  request(DFU_GETSTATE);
  return state;
}

{enum dfu_status, enum dfu_state, unsigned} dfu_getstatus(void)
{
  request(DFU_GETSTATUS);

  // special treament for the sync states:
  // make it look like we've stayed in the busy state (either dfuDNBUSY or
  // dfuMANIFEST) for the duration of poll timeout, while we actually leave
  // immediately (going back to the sync state)
  if (state == DFU_DNLOAD_SYNC)
    return {status, DFU_DNBUSY, POLL_TIMEOUT_MS};
  else if (state == DFU_MANIFEST_SYNC)
    return {status, DFU_MANIFEST, POLL_TIMEOUT_MS};

  return {status, state, 0};
}

void dfu_clrstatus(void)
{
  request(DFU_CLRSTATUS);
}

void dfu_detach(void)
{
  request(DFU_DETACH);
}

void dfu_bus_reset(fl_QSPIPorts &ports, const fl_QuadDeviceSpec spec[1])
{
  bus_reset(ports, spec);
}

void dfu_timeout_detach(void)
{
  timeout_detach();
}

void dfu_dnload(unsigned short block_num, size_t block_size_bytes,
                const char block[DFU_BLOCK_SIZE_MAX_BYTES])
{
  request_with_arguments(DFU_DNLOAD, block, null, block_size_bytes);
}
