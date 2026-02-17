// Copyright 2019-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <xs1.h>
#include <print.h>
#include <string.h>
#include <stdint.h>

#define DEBUG_UNIT DFU
#define DEBUG_PRINT_ENABLE_DFU 0
#include "debug_print.h"
#include "xassert.h"

#include "dfu_flash.h"
#include "dfu.h"
#include "fifo.h"

// TODOs
// Find place to insert flash_init() and flash_deinit()

#define POLL_TIMEOUT_MSEC 1

static enum dfu_state state = STATE_APP_IDLE;
static enum dfu_status status = DFU_OK;
static int32_t error_info = 0;

static struct fifo dfu_fifo;
static uint8_t dfu_fifo_storage[DFU_FLASH_PAGE_SIZE_BYTES];

enum dnload_sub_state {
  DNLOAD_SYNC,
  DNLOAD_ERASING,
  DNLOAD_WRITING
};

#if DEBUG_PRINT_ENABLE_DFU
static const char * unsafe request_str(enum dfu_request r)
{
  unsafe {
    switch (r) {
      case DFU_DETACH:                return "DETACH";
      case DFU_DNLOAD:                return "DNLOAD";
      case DFU_UPLOAD:                return "UPLOAD";
      case DFU_GETSTATUS:             return "GETSTATUS";
      case DFU_CLRSTATUS:             return "CLRSTATUS";
      case DFU_GETSTATE:              return "GETSTATE";
      case DFU_ABORT:                 return "ABORT";
      case XMOS_REBOOT:               return "XMOS_REBOOT";
      case XMOS_GET_ERROR_INFO:       return "XMOS_GET_ERROR_INFO";
      case XMOS_BUS_RESET:            return "XMOS_BUS_RESET";
      // TODO - add XUA custom requests here when defined
      default:                        return "?";
    }
  }
}

static const char * unsafe state_str(enum dfu_state s)
{
  unsafe {
    switch (s) {
      case STATE_APP_IDLE:                  return "appIDLE";
      case STATE_APP_DETACH:                return "appDETACH";
      case STATE_DFU_IDLE:                  return "dfuIDLE";
      case STATE_DFU_DOWNLOAD_SYNC:         return "dfuDNLOAD-SYNC";
      case STATE_DFU_DOWNLOAD_BUSY:         return "dfuDNBUSY";
      case STATE_DFU_DOWNLOAD_IDLE:         return "dfuDNLOAD-IDLE";
      case STATE_DFU_MANIFEST_SYNC:         return "dfuMANIFEST-SYNC";
      case STATE_DFU_MANIFEST:              return "dfuMANIFEST";
      case STATE_DFU_MANIFEST_WAIT_RESET:   return "dfuMANIFEST-WAIT-RESET";
      case STATE_DFU_UPLOAD_IDLE:           return "dfuUPLOAD-IDLE";
      case STATE_DFU_ERROR:                 return "dfuERROR";
      default:                              return "?";
    }
  }
}

static const char * unsafe dnload_sub_state_str(enum dnload_sub_state s)
{
  unsafe {
    switch (s) {
      case DNLOAD_SYNC:                     return "SYNC";
      case DNLOAD_ERASING:                  return "ERASING";
      case DNLOAD_WRITING:                  return "WRITING";
      default:                              return "?";
    }
  }
}

static const char * unsafe status_str(enum dfu_status s)
{
  unsafe {
    switch (s) {
      case DFU_OK:                      return "OK";
      case DFU_errTARGET:               return "errTARGET";
      case DFU_errFILE:                 return "errFILE";
      case DFU_errWRITE:                return "errWRITE";
      case DFU_errERASE:                return "errERASE";
      case DFU_errCHECK_ERASED:         return "errCHECK_ERASED";
      case DFU_errPROG:                 return "errPROG";
      case DFU_errVERIFY:               return "errVERIFY";
      case DFU_errADDRESS:              return "errADDRESS";
      case DFU_errNOTDONE:              return "errNOTDONE";
      case DFU_errFIRMWARE:             return "errFIRMWARE";
      case DFU_errVENDOR:               return "errVENDOR";
      case DFU_errUSBR:                 return "errUSBR";
      case DFU_errPOR:                  return "errPOR";
      case DFU_errUNKNOWN:              return "errUNKNOWN";
      case DFU_errSTALLED_PKT:          return "errSTALLEDPKT";
      default:                          return "?";
    }
  }
}
#endif

static void normal_transition(enum dfu_state new)
{
  unsafe {
    debug_printf("DFU: %s -> %s\n", state_str(state), state_str(new));
  }
  status = DFU_OK;
  state = new;
}

static void error_condition(enum dfu_status code, int32_t extra)
{
  unsafe {
    debug_printf("DFU: %s -> DFU_ERROR (%s %d)\n",
                 state_str(state), status_str(code), extra);
  }
  status = code;
  state = STATE_DFU_ERROR;
  error_info = extra;
}

static void sub_transition_dnload(enum dnload_sub_state new, enum dnload_sub_state &sub_state_arg)
{
  unsafe {
    debug_printf("DFU DNLOAD: %s -> %s\n",
                 dnload_sub_state_str(sub_state_arg),
                 dnload_sub_state_str(new));
  }
  sub_state_arg = new;
}

static int32_t dfufifo_full(void)
{
  // TODO - check whether fifo_is_full is sensible check, if fifo size and transfer size are not multiples of each 
  // other there could be some edge cases where this doesn't work as expected
  return fifo_is_full(dfu_fifo);
}

static enum dfu_status getstatus_from_dnload(enum dnload_sub_state &sub_state_arg)
{
  int32_t page_size_bytes = flash_get_page_size();
  uint8_t page[DFU_FLASH_PAGE_SIZE_BYTES];
  
  switch (sub_state_arg) {
    case DNLOAD_SYNC:
      // TODO - replace FLASH_MAX_UPGRADE_SIZE with image size from first page downloaded
      enum flash_status erase_status = flash_erase_sector_async(FLASH_MAX_UPGRADE_SIZE);
      if (erase_status != DFU_FLASH_OK && erase_status != DFU_FLASH_BUSY) {
        return DFU_errERASE;
      }

      sub_transition_dnload(DNLOAD_ERASING, sub_state_arg);

      break;

    case DNLOAD_ERASING:
      // TODO - replace FLASH_MAX_UPGRADE_SIZE with image size from first page downloaded
      enum flash_status erase_status = flash_erase_sector_async(FLASH_MAX_UPGRADE_SIZE);

      if (erase_status == DFU_FLASH_OK) {
        // sector erase completed, move on to page write
        sub_transition_dnload(DNLOAD_WRITING, sub_state_arg);

        if (fifo_block_dequeue(dfu_fifo, page, page_size_bytes) == FIFO_OK) {
          if (flash_write_page(page, page_size_bytes) != DFU_FLASH_OK) {
            return DFU_errWRITE;
          }
        }

      } else if (erase_status == DFU_FLASH_BUSY) {
        // still erasing, remain in this state and wait for next poll
      } else {
        return DFU_errERASE;
      }
      break;

    case DNLOAD_WRITING:
      if (fifo_block_dequeue(dfu_fifo, page, page_size_bytes) == FIFO_OK) {
        if (flash_write_page(page, page_size_bytes) != DFU_FLASH_OK) {
          return DFU_errWRITE;
        }
      }
      break;

      default:
        return DFU_errUNKNOWN;
  }

  return DFU_OK;
}

static enum dfu_status getstatus_from_manifest(void)
{
  int32_t page_size_bytes = flash_get_page_size();
  uint8_t page[DFU_FLASH_PAGE_SIZE_BYTES];

  if (page_size_bytes > DFU_FLASH_PAGE_SIZE_BYTES) {
    // sanity check - this should never happen
    return DFU_errUNKNOWN;
  }

  // drain conversion buffer of partial page, if any
  int32_t remaining_bytes = fifo_size(dfu_fifo);
  // TODO - this assumes that fifo is page sized.
  // If this is not the case, we may need to do multiple dequeues to drain the fifo.
  if (fifo_block_dequeue(dfu_fifo, page, remaining_bytes) == FIFO_OK) {
    memset(&page[remaining_bytes], 0xFF, page_size_bytes - remaining_bytes);
    if (flash_write_page(page, page_size_bytes) != DFU_FLASH_OK) {
      return DFU_errWRITE;
    }
  }
  return DFU_OK;
}

/* Expecting only to be called from DFU_IDLE or DFU_DNLOAD_IDLE, fifo should be ready */
static int32_t dnload_block(const uint8_t write_block[], int32_t block_num, int32_t block_size_bytes)
{
  (void)block_num; // TODO - remove

  if (block_size_bytes > 0) {
    // non-zero return value from the push function indicates not enough space
    // in the queue of blocks awaiting conversion to pages
    // for some reason there are have been not enough pulls or too many pushes
    if (fifo_block_enqueue(dfu_fifo, write_block, block_size_bytes) != FIFO_OK) {
      return 3;
    }
  }

  return 0;
}

static int32_t upload_block(uint8_t read_block[], int32_t block_size_bytes)
{
  // TODO - fix - add fifo here to buffer data reads. Part of wider upload feature fix.
  if (block_size_bytes > 0) {
    if (flash_read_page(read_block, block_size_bytes) != DFU_FLASH_OK) {
      return -1;
    }
  }
  return 0;
}

static void request_with_arguments(enum dfu_request request,
                                   const uint8_t (&?write_block)[DFU_TRANSFER_SIZE_BYTES],
                                   uint8_t (&?read_block)[DFU_TRANSFER_SIZE_BYTES],
                                   int32_t block_size_bytes, int32_t &?block_num)
{
  static enum dnload_sub_state sub_state = DNLOAD_SYNC;
  static int32_t read_block_num = 0;
  static int32_t read_length = 0;

#if DEBUG_PRINT_ENABLE_DFU
  debug_printf("DFU: %s", request_str(request));
  if (request == DFU_DNLOAD) {
    debug_printf(" 0x%X %d\n", block_num, block_size_bytes);
  } else {
    debug_printf("\n");
  }
#endif
  enum dfu_status rqst_status;
  int32_t ret;

  switch (state) {
    case STATE_APP_IDLE:
      if (request == DFU_DETACH) {
        normal_transition(STATE_APP_DETACH);
      }
      // no other requests expected, stay in appIDLE
      break;

    case STATE_APP_DETACH:
      if (request != DFU_GETSTATUS && request != DFU_GETSTATE) {
        // no other requests expected, return to appIDLE
        normal_transition(STATE_APP_IDLE);
        sub_state = DNLOAD_SYNC;
      }
      break;

    case STATE_DFU_IDLE:
      if (request == DFU_DNLOAD) {
        if (!flash_is_connected()) {
          if (flash_init() != DFU_FLASH_OK) {
            error_condition(DFU_errTARGET, 0);
            break;
          }
        }
        fifo_init(dfu_fifo, dfu_fifo_storage, sizeof(dfu_fifo_storage));
        ret = dnload_block(write_block, block_num, block_size_bytes);
        // TODO - test first page for valid image and return errFILE if not valid
        if (ret != 0) {
          error_condition(DFU_errUNKNOWN, ret);
        } else {
          normal_transition(STATE_DFU_DOWNLOAD_SYNC);
        }

      } else if (request == DFU_UPLOAD) {
        if (!flash_is_connected()) {
          if (flash_init() != DFU_FLASH_OK) {
            error_condition(DFU_errTARGET, 0);
            break;
          }
          read_block_num = 0;
          
          struct flash_data_status start_status = flash_start_read();
          if (start_status.status != DFU_FLASH_OK) {
            error_condition(DFU_errFILE, 0);

          } else {
            read_length = start_status.data;
            // TODO - for no-clock-stretching we may have to read out-of-band
            int32_t upload = upload_block(read_block, block_size_bytes);
            if (upload != 0) {
              error_condition(DFU_errFILE, upload);
            } else {
              normal_transition(STATE_DFU_UPLOAD_IDLE);
            }
          }
        } else {
          // it is an error if flash is aready connected. Something has not cleaned up.
          error_condition(DFU_errTARGET, 0);
        }
        
      } else if (request != DFU_GETSTATUS && request != DFU_GETSTATE) {
        // no other requests expected, defined as error
        error_condition(DFU_errSTALLED_PKT, request);
      }
      break;

    case STATE_DFU_DOWNLOAD_SYNC:
      if (request == DFU_GETSTATUS) {
        rqst_status = getstatus_from_dnload(sub_state);
        if (rqst_status != DFU_OK) {
          error_condition(rqst_status, 0);

        } else {
          if (dfufifo_full()) {
            normal_transition(STATE_DFU_DOWNLOAD_BUSY);
            normal_transition(STATE_DFU_DOWNLOAD_SYNC);

          } else {
            normal_transition(STATE_DFU_DOWNLOAD_IDLE);
          }
        }
      }
      else if (request != DFU_GETSTATE) {
        error_condition(DFU_errSTALLED_PKT, request);
      }
      break;

    case STATE_DFU_MANIFEST_SYNC:
      if (request == DFU_GETSTATUS) {
        rqst_status = getstatus_from_manifest();
        if (rqst_status != DFU_OK) {
          error_condition(rqst_status, 0);

        } else {
          if (fifo_is_empty(dfu_fifo)) {
            flash_finalise_write();
            normal_transition(STATE_DFU_IDLE);
            sub_state = DNLOAD_SYNC;
            flash_deinit();

          } else {
            normal_transition(STATE_DFU_MANIFEST);
            normal_transition(STATE_DFU_MANIFEST_SYNC);
          }
        }
      }
      else if (request != DFU_GETSTATE) {
        error_condition(DFU_errSTALLED_PKT, request);
      }
      break;

    case STATE_DFU_DOWNLOAD_IDLE:
      if (request == DFU_DNLOAD) {
        if (block_size_bytes == 0) {
          ret = dnload_block(write_block, 0, 0);
          if (ret != 0) {
            error_condition(DFU_errFILE, ret);
          } else {
            normal_transition(STATE_DFU_MANIFEST_SYNC);
          }

        } else {
          ret = dnload_block(write_block, block_num, block_size_bytes);
          if (ret != 0) {
            error_condition(DFU_errWRITE, ret);
          } else {
            normal_transition(STATE_DFU_DOWNLOAD_SYNC);
          }
        }
      } else if (request != DFU_GETSTATUS && request != DFU_GETSTATE) {
        error_condition(DFU_errSTALLED_PKT, request);
      }
      break;

    case STATE_DFU_UPLOAD_IDLE:
      if (request == DFU_UPLOAD) {
        // TODO - use read_length to terminate read correctly.
        // TODO - figure out transition to DFU_IDLE

        int32_t upload = upload_block(read_block, block_size_bytes);
        if (upload != 0) {
          error_condition(DFU_errFILE, upload);
        } else {
          block_num = read_block_num;
          read_block_num += 1;
          // normal_transition(STATE_DFU_UPLOAD_IDLE);
        }
        // flash_deinit() and DFU_IDLE
      } else if (request != DFU_GETSTATUS && request != DFU_GETSTATE) {
        // no other requests expected, defined as error
        error_condition(DFU_errSTALLED_PKT, request);
      }
      break;

    case STATE_DFU_ERROR:
      if (request == DFU_CLRSTATUS) {
        normal_transition(STATE_DFU_IDLE);
        // TODO - confirm this is correct
        sub_state = DNLOAD_SYNC;
      }
      break;
  }
}

static void request(enum dfu_request request)
{
  request_with_arguments(request, null, null, 0, null);
}

enum dfu_state dfu_getstate(void)
{
  request(DFU_GETSTATE);
  return state;
}

struct dfu_getstatus dfu_getstatus(void)
{
  request(DFU_GETSTATUS);

  struct dfu_getstatus ret;
  ret.status = status;
  ret.state = state;
  ret.poll_timeout_msec = POLL_TIMEOUT_MSEC;

  // special treament for the sync states:
  // make it look like we've stayed in the busy state (either dfuDNBUSY or
  // dfuMANIFEST) for the duration of poll timeout, while we actually leave
  // immediately (going back to the sync state)
  if (state == STATE_DFU_DOWNLOAD_SYNC) {
    ret.state = STATE_DFU_DOWNLOAD_BUSY;
  } else if (state == STATE_DFU_MANIFEST_SYNC) {
    ret.state = STATE_DFU_MANIFEST;
  } else {
    ret.poll_timeout_msec = 0;
  }

  return ret;
}

void dfu_clrstatus(void)
{
  request(DFU_CLRSTATUS);
}

void dfu_detach(void)
{
  request(DFU_DETACH);
}

void dfu_bus_reset(void)
{
  if (state == STATE_APP_DETACH) {
    normal_transition(STATE_DFU_IDLE);
  } else if (state == STATE_APP_IDLE) {
    normal_transition(STATE_APP_IDLE);
  } else {
    error_condition(DFU_errUSBR, state);
  }
}

void dfu_timeout_detach(void)
{
  if (state == STATE_APP_DETACH) {
    normal_transition(STATE_APP_IDLE);
  }
  else {
    debug_printf("unexpected detach timeout call\n");
    // remain in current state, no error code indication
  }
}

void dfu_dnload(int32_t block_num, size_t block_size_bytes, const uint8_t write_block[DFU_TRANSFER_SIZE_BYTES])
{
  request_with_arguments(DFU_DNLOAD, write_block, null,  block_size_bytes, block_num);
}

int32_t dfu_upload(size_t block_size_bytes, uint8_t read_block[DFU_TRANSFER_SIZE_BYTES])
{
  int32_t block_num = 0;
  request_with_arguments(DFU_UPLOAD, null, read_block, block_size_bytes, block_num);
  return block_num;
}

int32_t dfu_get_error_info(void)
{
  return error_info;
}
