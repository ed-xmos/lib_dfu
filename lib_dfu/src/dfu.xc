// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <print.h>

#define _Bool int
#include <stdbool.h>

#define DEBUG_UNIT DFU
#define DEBUG_PRINT_ENABLE_DFU 0
#include "debug_print.h"

#include "dfu_buffer_converter.h"
#include "dfu_flash.h"
#include "dfu.h"

#define POLL_TIMEOUT_MS 1

static enum dfu_state state = APP_IDLE;
static enum dfu_status status = DFU_OK;

static struct buffer_converter converter;

static unsigned page_size_bytes = 0;
static unsigned boot_upgrade_slot_start = 0;
static unsigned data_upgrade_slot_start = 0;

static struct {
  int next_page_address;
  char page[DFU_PAGE_SIZE_MAX_BYTES];
  bool page_ready;
  enum dnload_sub_state {
    DNLOAD_SYNC,
    DNLOAD_ERASING_SECTOR,
    DNLOAD_WRITING_PAGE
  } sub_state;
} dnload = {0, {}, false, DNLOAD_SYNC};

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
      default:                        return "?";
    }
  }
}

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

static const char * unsafe dnload_sub_state_str(enum dnload_sub_state s)
{
  unsafe {
    switch (s) {
      case DNLOAD_SYNC:               return "SYNC";
      case DNLOAD_ERASING_SECTOR:     return "ERASING_SECTOR";
      case DNLOAD_WRITING_PAGE:       return "WRITING_PAGE";
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
      default:                        return "?";
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

static void error_condition(enum dfu_status code)
{
  unsafe {
    debug_printf("DFU: %s -> DFU_ERROR (%s)\n", state_str(state), status_str(code));
  }
  status = code;
  state = DFU_ERROR;
}

static void sub_transition_dnload(enum dnload_sub_state new)
{
  unsafe {
    debug_printf("DFU DNLOAD: %s -> %s\n",
                 dnload_sub_state_str(dnload.sub_state),
                 dnload_sub_state_str(new));
  }
  dnload.sub_state = new;
}

static enum dfu_status getstatus_from_dnload(bool &busy)
{
  busy = true;

  switch (dnload.sub_state) {
    case DNLOAD_SYNC:
      if (dnload.page_ready) {
        if (flash_is_first_whole_page_in_sector(dnload.next_page_address)) {
          sub_transition_dnload(DNLOAD_ERASING_SECTOR);
          if (flash_erase_sector_async(dnload.next_page_address) != 0)
            return ERR_ERASE;
        }
        else {
          sub_transition_dnload(DNLOAD_WRITING_PAGE);
          if (flash_write_page_async(dnload.next_page_address, dnload.page) != 0)
            return ERR_WRITE;
        }
      }
      else {
        busy = false;
      }
      break;

    case DNLOAD_ERASING_SECTOR:
      if (!flash_is_busy()) {
        if (!flash_is_sector_erased(dnload.next_page_address))
          return ERR_CHECK_ERASED;

        sub_transition_dnload(DNLOAD_WRITING_PAGE);
        if (flash_write_page_async(dnload.next_page_address, dnload.page) != 0)
          return ERR_WRITE;
      }
      break;

    case DNLOAD_WRITING_PAGE:
      if (!flash_is_busy()) {
        if (flash_verify_page(dnload.next_page_address, dnload.page) != 0)
          return ERR_VERIFY;

        sub_transition_dnload(DNLOAD_SYNC);

        dnload.next_page_address += page_size_bytes;

        if (buffer_converter_pull(converter, dnload.page, page_size_bytes) != 0) {
          dnload.page_ready = false;
          busy = false;
        }
      }
      break;
  }

  return DFU_OK;
}

static int dnload_block(const char write_block[], int block_num, int block_size_bytes)
{
  // it should be an error for the sub-state machine to go out of sync
  // eg host omitting a GETSTATUS request
  if (dnload.sub_state != DNLOAD_SYNC)
    return 1;

  // there should never be an unprocessed page when DNLOAD request is sent
  // an unprocessed page is written out first with repeated GETSTATUS requests
  if (dnload.page_ready)
    return 2;

  if (block_size_bytes > 0) {
    // find slot start only once we know that the required operation is DNLOAD
    if (block_num & 0x8000) {
      if (data_upgrade_slot_start == 0) {
        if (flash_locate_data_upgrade_slot(data_upgrade_slot_start) != 0)
          return 3;

        debug_printf("DFU: data upgrade slot start 0x%X\n", data_upgrade_slot_start);
      }
    }
    else {
      if (boot_upgrade_slot_start == 0) {
        if (flash_locate_upgrade_slot(boot_upgrade_slot_start) != 0)
          return 4;

        debug_printf("DFU: boot upgrade slot start 0x%X\n", boot_upgrade_slot_start);
      }
    }

    // peek at main state here to determine if this is the first DNLOAD bloc of
    // a given operation so we can suitably start things off
    if (state == DFU_IDLE) {
      dnload.next_page_address = block_num & 0x8000 ? data_upgrade_slot_start :
                                                      boot_upgrade_slot_start;
      buffer_converter_reset(converter);
    }

    // non-zero return value from the push function indicates not enough space
    // in the queue of blocks awaiting conversion to pages
    // for some reason there are have been not enough pulls or too many pushes
    if (buffer_converter_push(converter, write_block, block_size_bytes) != 0)
      return 5;

    // normal scenario: once we have enough blocks to make one page, commit this
    // page for the next stage: optional sector erase followed by one or more
    // page writes
    if (buffer_converter_pull(converter, dnload.page, page_size_bytes) == 0)
      dnload.page_ready = true;
  }

  if (block_size_bytes == 0) {
    // drain conversion buffer of partial page, if any
    if (buffer_converter_padded_pull(converter, dnload.page, page_size_bytes) > 0)
      dnload.page_ready = true;
  }

  return 0;
}

static void request_with_arguments(enum dfu_request request,
                                   const char (&?write_block)[DFU_BLOCK_SIZE_MAX_BYTES],
                                   char (&?read_block)[DFU_BLOCK_SIZE_MAX_BYTES],
                                   int block_size_bytes, int write_block_num)
{
#if DEBUG_PRINT_ENABLE_DFU
  debug_printf("DFU: %s", request_str(request));
  if (request == DFU_DNLOAD)
    debug_printf(" 0x%X %d\n", write_block_num, block_size_bytes);
  else
    debug_printf("\n");
#endif
  enum dfu_status status;

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
        if (dnload_block(write_block, write_block_num, block_size_bytes) != 0)
          error_condition(ERR_UNKNOWN);
        else
          normal_transition(DFU_DNLOAD_SYNC);
      }
      else if (request != DFU_GETSTATUS && request != DFU_GETSTATE) {
        // no other requests expected, defined as error
        error_condition(ERR_STALLED_PKT);
      }
      break;

    case DFU_DNLOAD_SYNC:
      if (request == DFU_GETSTATUS) {
        bool busy = false;
        status = getstatus_from_dnload(busy);
        if (status != DFU_OK) {
          error_condition(status);
        }
        else {
          if (busy) {
            normal_transition(DFU_DNBUSY);
            normal_transition(DFU_DNLOAD_SYNC);
          }
          else {
            normal_transition(DFU_DNLOAD_IDLE);
          }
        }
      }
      break;

    case DFU_MANIFEST_SYNC:
      if (request == DFU_GETSTATUS) {
        bool busy = false;
        status = getstatus_from_dnload(busy);
        if (status != DFU_OK) {
          error_condition(status);
        }
        else {
          if (busy) {
            normal_transition(DFU_MANIFEST);
            normal_transition(DFU_MANIFEST_SYNC);
          }
          else {
            normal_transition(DFU_IDLE);
            // not disconnecting from flash to allow additional operations
            if (flash_set_write_disable() != 0)
              error_condition(ERR_WRITE);
          }
        }
      }
      else if (request != DFU_GETSTATE) {
        error_condition(ERR_STALLED_PKT);
      }
      break;

    case DFU_DNLOAD_IDLE:
      if (block_size_bytes == 0) {
        if (dnload_block(write_block, 0, 0) != 0)
          error_condition(ERR_UNKNOWN);
        else
          normal_transition(DFU_MANIFEST_SYNC);
      }
      else {
        if (dnload_block(write_block, write_block_num, block_size_bytes) != 0)
          error_condition(ERR_UNKNOWN);
        else
          normal_transition(DFU_DNLOAD_SYNC);
      }
      break;

    case DFU_ERROR:
      if (request == DFU_CLRSTATUS) {
        normal_transition(DFU_IDLE);
      }
      break;
  }
}

static void request(enum dfu_request request)
{
  request_with_arguments(request, null, null, 0, 0);
}

static enum dfu_status enter_dfu(fl_QSPIPorts &ports, const fl_QuadDeviceSpec spec[1])
{
  int ret;

  ret = flash_connect(ports, spec);
  if (ret != 0) {
    debug_printf("error: quadflash connectToDevice returned %d\n", ret);
    return ERR_UNKNOWN;
  }

  page_size_bytes = spec[0].pageSize;
  if (page_size_bytes > DFU_PAGE_SIZE_MAX_BYTES)
    return ERR_UNKNOWN;

  // only support regular sector layout
  if (spec[0].sectorLayout != SECTOR_LAYOUT_REGULAR)
    return ERR_UNKNOWN;

  // only support erase of exact sector size
  if (spec[0].sectorEraseSize != spec[0].sectorSizes.regularSectorSize)
    return ERR_UNKNOWN;

  return DFU_OK;
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
  if (state == APP_DETACH) {
    enum dfu_status status = enter_dfu(ports, spec);
    if (status != DFU_OK)
      error_condition(status);
    else
      normal_transition(DFU_IDLE);
  }
  else if (state == APP_IDLE) {
    normal_transition(APP_IDLE);
  }
  else {
    error_condition(ERR_USBR);
  }
}

void dfu_timeout_detach(void)
{
  if (state == APP_DETACH) {
    normal_transition(APP_IDLE);
  }
  else {
    debug_printf("unexpected detach timeout call\n");
    // remain in current state, no error code indication
  }
}

void dfu_dnload(unsigned short block_num, size_t block_size_bytes,
                const char block[DFU_BLOCK_SIZE_MAX_BYTES])
{
  request_with_arguments(DFU_DNLOAD, block, null, block_size_bytes, block_num);
}
