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
#include "dfu_reboot.h"
#include "fifo.h"

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

      case XMOS_BUS_RESET:            return "XMOS_BUS_RESET";
      
      case XMOS_DFU_REVERTFACTORY:    return "XMOS_DFU_REVERTFACTORY";
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

static struct dfu_cmd_response normal_transition(enum dfu_state new)
{
  unsafe {
    debug_printf("DFU: %s -> %s\n", state_str(state), state_str(new));
  }
  status = DFU_OK;
  state = new;
  struct dfu_cmd_response response = { DFU_API_SUCCESS, 0, DFU_RESET_TYPE_NONE };
  return response;
}

static struct dfu_cmd_response error_condition(enum dfu_status code, int32_t extra)
{
  unsafe {
    debug_printf("DFU: %s -> DFU_ERROR (%s %d)\n",
                 state_str(state), status_str(code), extra);
  }
  status = code;
  state = STATE_DFU_ERROR;
  error_info = extra;
  struct dfu_cmd_response response = { DFU_API_ERROR, 0, DFU_RESET_TYPE_NONE };
  return response;
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
static enum dfu_api_status dnload_block(const uint8_t write_block[], int32_t block_num, int32_t block_size_bytes)
{
  (void)block_num; // TODO - remove, or check
  enum dfu_api_status dnload_status = DFU_API_ERROR;

  if (block_size_bytes > 0) {
    // non-zero return value from the push function indicates not enough space
    // in the queue of blocks awaiting conversion to pages
    // for some reason there are have been not enough pulls or too many pushes
    if (fifo_block_enqueue(dfu_fifo, write_block, block_size_bytes) == FIFO_OK) {
      dnload_status = DFU_API_SUCCESS;
    }
  } else {
    // zero-size block indicates end of download, move on to manifest state
    dnload_status = DFU_API_SUCCESS;
  }

  return dnload_status;
}

static enum dfu_api_status upload_block(uint8_t read_block[], int32_t block_size_bytes)
{
  if (fifo_is_empty(dfu_fifo)) {
    if (flash_read_page(dfu_fifo_storage, DFU_FLASH_PAGE_SIZE_BYTES) != DFU_FLASH_OK) {
      return DFU_API_ERROR;
    } else {
      // TODO - fix snooping into fifo
      fifo_init(dfu_fifo, dfu_fifo_storage, sizeof(dfu_fifo_storage));
      dfu_fifo.count = dfu_fifo.max;
    }
  }

  if (fifo_block_dequeue(dfu_fifo, read_block, block_size_bytes) == FIFO_OK) {
    return DFU_API_SUCCESS;
  }

  return DFU_API_ERROR;
}

static struct dfu_cmd_response state_app_idle(enum dfu_request request) {
  struct dfu_cmd_response response = { DFU_API_BAD_PARAM, 0, DFU_RESET_TYPE_NONE };
  if (request == XMOS_BUS_RESET) {
    response.status = DFU_API_SUCCESS;
    // TODO - USB DFU mode enable when "value" is set.
    // response = normal_transition(STATE_DFU_IDLE);
    // response.reset_type = DFU_RESET_TYPE_RESET_TO_DFU;

  } else if (request == DFU_DETACH) {
    response = normal_transition(STATE_APP_DETACH);

  } else if (request == DFU_ABORT) {
    response.status = DFU_API_SUCCESS;

  } else if (request != DFU_GETSTATUS && request != DFU_GETSTATE) {
    // no other requests expected, defined as error
    response = error_condition(DFU_errSTALLED_PKT, request);
  }
  // no other requests expected, stay in appIDLE
  return response;
}

static struct dfu_cmd_response state_detach(enum dfu_request request) {
  struct dfu_cmd_response response = { DFU_API_BAD_PARAM, 0, DFU_RESET_TYPE_NONE };
  if (request == XMOS_BUS_RESET) {
    response = normal_transition(STATE_DFU_IDLE);

  } else if (request != DFU_GETSTATUS && request != DFU_GETSTATE) {
    // no other requests expected, return to appIDLE, but respond with STALL.
    response = normal_transition(STATE_APP_IDLE);
    response.status = DFU_API_ERROR;
  }
  return response;
}

static struct dfu_cmd_response state_entry_dnload(const uint8_t (&?write_block)[DFU_TRANSFER_SIZE_BYTES],
                                                  int32_t block_size_bytes, int32_t &?block_num) {
  struct dfu_cmd_response response = { DFU_API_BAD_PARAM, 0, DFU_RESET_TYPE_NONE };
  if (!flash_is_connected()) {
    if (flash_init() != DFU_FLASH_OK) {
      response = error_condition(DFU_errTARGET, 0);
      return response;
    }
  }
  fifo_init(dfu_fifo, dfu_fifo_storage, sizeof(dfu_fifo_storage));
  enum dfu_api_status ret = dnload_block(write_block, block_num, block_size_bytes);
  // TODO - test first page for valid image and return errFILE if not valid
  if (ret != DFU_API_SUCCESS) {
    response = error_condition(DFU_errUNKNOWN, ret);
  } else {
    response = normal_transition(STATE_DFU_DOWNLOAD_SYNC);
  }
  return response;
}

static struct dfu_cmd_response state_dnload_sync(enum dfu_request request, enum dnload_sub_state &sub_state_arg) {
  struct dfu_cmd_response response = { DFU_API_BAD_PARAM, 0, DFU_RESET_TYPE_NONE };
  if (request == DFU_GETSTATUS) {
    enum dfu_status rqst_status = getstatus_from_dnload(sub_state_arg);
    if (rqst_status != DFU_OK) {
      response = error_condition(rqst_status, 0);

    } else {
      if (dfufifo_full()) {
        response = normal_transition(STATE_DFU_DOWNLOAD_BUSY);
        response = normal_transition(STATE_DFU_DOWNLOAD_SYNC);

      } else {
        response = normal_transition(STATE_DFU_DOWNLOAD_IDLE);
      }
    }
  }
  else if (request != DFU_GETSTATE) {
    response = error_condition(DFU_errSTALLED_PKT, request);
  }
  return response;
}

static struct dfu_cmd_response state_manifest_sync(enum dfu_request request, enum dnload_sub_state &sub_state_arg) {
  struct dfu_cmd_response response = { DFU_API_BAD_PARAM, 0, DFU_RESET_TYPE_NONE };
  if (request == DFU_GETSTATUS) {
    enum dfu_status rqst_status = getstatus_from_manifest();
    if (rqst_status != DFU_OK) {
      response = error_condition(rqst_status, 0);

    } else {
      if (fifo_is_empty(dfu_fifo)) {
        flash_finalise_write();
        response = normal_transition(STATE_DFU_IDLE);
        sub_state_arg = DNLOAD_SYNC;
        flash_deinit();

      } else {
        response = normal_transition(STATE_DFU_MANIFEST);
        response = normal_transition(STATE_DFU_MANIFEST_SYNC);
      }
    }
  }
  else if (request != DFU_GETSTATE) {
    response = error_condition(DFU_errSTALLED_PKT, request);
  }
  return response;
}
static struct dfu_cmd_response state_download_idle(const uint8_t (&?write_block)[DFU_TRANSFER_SIZE_BYTES],
                                                  int32_t block_size_bytes, int32_t &?block_num) {
  struct dfu_cmd_response response = { DFU_API_BAD_PARAM, 0, DFU_RESET_TYPE_NONE };
  if (block_size_bytes == 0) {
    enum dfu_api_status ret = dnload_block(write_block, 0, 0);
    if (ret != DFU_API_SUCCESS) {
      response = error_condition(DFU_errFILE, ret);
    } else {
      response = normal_transition(STATE_DFU_MANIFEST_SYNC);
    }

  } else {
    enum dfu_api_status ret = dnload_block(write_block, block_num, block_size_bytes);
    if (ret != DFU_API_SUCCESS) {
      response = error_condition(DFU_errWRITE, ret);
    } else {
      response = normal_transition(STATE_DFU_DOWNLOAD_SYNC);
    }
  }
  return response;
}

static struct dfu_cmd_response state_entry_upload(uint8_t (&?read_block)[DFU_TRANSFER_SIZE_BYTES],
                                                  int32_t block_size_bytes, int32_t &?read_length) {
  struct dfu_cmd_response response = { DFU_API_BAD_PARAM, 0, DFU_RESET_TYPE_NONE };
  if (!flash_is_connected()) {
    if (flash_init() != DFU_FLASH_OK) {
      response = error_condition(DFU_errTARGET, 0);
      return response;
    }

    fifo_init(dfu_fifo, dfu_fifo_storage, sizeof(dfu_fifo_storage));
    
    struct flash_data_status start_status = flash_start_read();
    if (start_status.status != DFU_FLASH_OK) {
      response = error_condition(DFU_errFILE, 0);

    } else {
      read_length = start_status.data;
      // TODO - for no-clock-stretching we may have to read out-of-band
      enum dfu_api_status upload = upload_block(read_block, block_size_bytes);
      if (upload != DFU_API_SUCCESS) {
        response = error_condition(DFU_errFILE, upload);
      } else {
        response = normal_transition(STATE_DFU_UPLOAD_IDLE);;
        response.return_data_len = (read_length < block_size_bytes) ? read_length : block_size_bytes;
        read_length -= block_size_bytes;
      }
    }
  } else {
    // it is an error if flash is aready connected. Something has not cleaned up.
    response = error_condition(DFU_errTARGET, 0);
  }
  return response;
}

static struct dfu_cmd_response state_upload_idle(uint8_t (&?read_block)[DFU_TRANSFER_SIZE_BYTES],
                                                 int32_t block_size_bytes, int32_t &?read_length) {
  struct dfu_cmd_response response = { DFU_API_BAD_PARAM, 0, DFU_RESET_TYPE_NONE };
  if (read_length <= 0) {
    // Terminate read
    response = normal_transition(STATE_DFU_IDLE);
    response.return_data_len = 0;
    flash_deinit();

  } else {
    enum dfu_api_status upload = upload_block(read_block, block_size_bytes);
    if (upload != DFU_API_SUCCESS) {
      response = error_condition(DFU_errFILE, upload);
    } else {
      if (read_length < block_size_bytes) {
        response = normal_transition(STATE_DFU_IDLE);
        response.return_data_len = read_length;
        flash_deinit();
      } else {
        response.status = DFU_API_SUCCESS;
        response.return_data_len = block_size_bytes;
      }
      read_length -= block_size_bytes;
    }
  }
  return response;
}

struct dfu_cmd_response request_with_arguments(enum dfu_request request,
                                              const uint8_t (&?write_block)[DFU_TRANSFER_SIZE_BYTES],
                                              uint8_t (&?read_block)[DFU_TRANSFER_SIZE_BYTES],
                                              int32_t block_size_bytes, int32_t &?block_num)
{
  static enum dnload_sub_state sub_state = DNLOAD_SYNC;
  static int32_t read_length = 0;
  struct dfu_cmd_response response = { DFU_API_BAD_PARAM, 0, DFU_RESET_TYPE_NONE };

#if DEBUG_PRINT_ENABLE_DFU
  debug_printf("DFU: %s", request_str(request));
  if (request == DFU_DNLOAD) {
    debug_printf(" 0x%X %d\n", block_num, block_size_bytes);
  } else {
    debug_printf("\n");
  }
#endif
  switch (state) {
    case STATE_APP_IDLE:
      response = state_app_idle(request);
      break;

    case STATE_APP_DETACH:
      response = state_detach(request);
      if (response.status == DFU_API_SUCCESS) {
        sub_state = DNLOAD_SYNC;
      }
      break;

    case STATE_DFU_IDLE:
      if (request == DFU_DNLOAD) {
        response = state_entry_dnload(write_block, block_size_bytes, block_num);

      } else if (request == DFU_UPLOAD) {
        response = state_entry_upload(read_block, block_size_bytes, read_length);

      } else if (request != DFU_GETSTATUS && request != DFU_GETSTATE) {
        // no other requests expected, defined as error
        response = error_condition(DFU_errSTALLED_PKT, request);
      }
      break;

    case STATE_DFU_DOWNLOAD_SYNC:
      response = state_dnload_sync(request, sub_state);
      break;

    case STATE_DFU_MANIFEST_SYNC:
      response = state_manifest_sync(request, sub_state);
      break;

    case STATE_DFU_DOWNLOAD_IDLE:
      if (request == DFU_DNLOAD) {
        response = state_download_idle(write_block, block_size_bytes, block_num);

      } else if (request != DFU_GETSTATUS && request != DFU_GETSTATE) {
        response = error_condition(DFU_errSTALLED_PKT, request);
      }
      break;

    case STATE_DFU_UPLOAD_IDLE:
      if (request == DFU_UPLOAD) {
        response = state_upload_idle(read_block, block_size_bytes, read_length);

      } else if (request != DFU_GETSTATUS && request != DFU_GETSTATE) {
        // no other requests expected, defined as error
        response = error_condition(DFU_errSTALLED_PKT, request);
      }
      break;

    case STATE_DFU_ERROR:
      if (request == DFU_CLRSTATUS) {
        response = normal_transition(STATE_DFU_IDLE);
        // TODO - confirm this is correct
        sub_state = DNLOAD_SYNC;
      }
      break;
  }

  /* Handle common requests last */
  if (request == DFU_GETSTATUS && !isnull(read_block) && block_size_bytes == DFU_GET_STATUS_PAYLOAD_SIZE_BYTES) {
    unsigned int ret_poll_timeout_msec = 0; // TODO - move?
    memset(read_block, 0, DFU_GET_STATUS_PAYLOAD_SIZE_BYTES);
    read_block[DFU_GETSTATUS_STATUS_INDEX] = status;
    memcpy(&read_block[DFU_GETSTATUS_POLL_TIMEOUT_INDEX], &ret_poll_timeout_msec, DFU_GETSTATUS_POLL_TIMEOUT_BYTES);

    // special treatment for the sync states: make it look like we've stayed in the busy state (either dfuDNBUSY or
    // dfuMANIFEST) for the duration of poll timeout, while we actually leave immediately (going back to the sync state)
    if (state == STATE_DFU_DOWNLOAD_SYNC) {
      read_block[DFU_GETSTATUS_STATE_INDEX] = STATE_DFU_DOWNLOAD_BUSY;
    } else if (state == STATE_DFU_MANIFEST_SYNC) {
      read_block[DFU_GETSTATUS_STATE_INDEX] = STATE_DFU_MANIFEST;
    } else {
      read_block[DFU_GETSTATUS_STATE_INDEX] = state;
    }
    
    response.status = DFU_API_SUCCESS;
    response.return_data_len = DFU_GET_STATUS_PAYLOAD_SIZE_BYTES;

  } else if (request == DFU_GETSTATE && !isnull(read_block) && block_size_bytes == DFU_GET_STATE_PAYLOAD_SIZE_BYTES) {
    response.status = DFU_API_SUCCESS;
    response.return_data_len = DFU_GET_STATE_PAYLOAD_SIZE_BYTES;
    response.reset_type = DFU_RESET_TYPE_NONE;
    read_block[DFU_GETSTATE_INDEX] = state;

  } else if ((request == XMOS_BUS_RESET) && (response.status != DFU_API_SUCCESS)) {
    // if bus reset was not handled by state machine handlers, handle it here by resetting to app idle.
    if (state != STATE_APP_IDLE) {
      /* Exit from DFU mode. Send reboot command */
      timer tmr;
      unsigned now;
      tmr :> now;
      tmr when timerafter(now + (DELAY_BEFORE_REBOOT_FROM_DFU_MS * XS1_TIMER_KHZ)) :> void;
      device_reboot();
      // Note: testing will fall through to app idle without reboot, which is fine.
    }
    response = normal_transition(STATE_APP_IDLE);

  } else {
    /* For other requests, delegate to state machine handlers */
  }

  return response;
}

struct dfu_cmd_response request(enum dfu_request request)
{
  return request_with_arguments(request, null, null, 0, null);
}

void dfu_bus_reset(void)
{
  request(XMOS_BUS_RESET);
}

void dfu_clrstatus(void)
{
  request(DFU_CLRSTATUS);
}

void dfu_detach(void)
{
  request(DFU_DETACH);
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

void dfu_dnload(int32_t block_num, int32_t block_size_bytes, const uint8_t write_block[DFU_TRANSFER_SIZE_BYTES])
{
  request_with_arguments(DFU_DNLOAD, write_block, null, block_size_bytes, block_num);
}

int32_t dfu_upload(int32_t block_size_bytes, uint8_t read_block[DFU_TRANSFER_SIZE_BYTES])
{
  struct dfu_cmd_response response = request_with_arguments(DFU_UPLOAD, null, read_block, block_size_bytes, null);
  return (response.status == DFU_API_SUCCESS) ? response.return_data_len : -1;
}
