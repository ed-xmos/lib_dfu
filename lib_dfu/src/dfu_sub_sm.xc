// Copyright 2011-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "dfu_sub_sm.h"

#include <xs1.h>
#include <platform.h>
#include <string.h>

#define DEBUG_UNIT DFU_PROFILER
#define DEBUG_PRINT_ENABLE_DFU_PROFILER 1
#include "debug_print.h"
#include "xassert.h"

#include "dfu.h"
#include "dfu_flash.h"
#include "fifo.h"


static timer t_profiler;
static unsigned t_profiler_start = 0;
static unsigned t_profiler_end = 0;

static unsigned t_profile_connect = 0;
static unsigned t_profile_first_erase = 0;
static unsigned t_profile_first_write = 0;
// static unsigned t_profile_second_write = 0;

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

static void sub_transition_dnload(enum dnload_sub_state new, enum dnload_sub_state &sub_state_arg)
{
  unsafe {
    debug_printf("DFU DNLOAD: %s -> %s\n",
                 dnload_sub_state_str(sub_state_arg),
                 dnload_sub_state_str(new));
  }
  sub_state_arg = new;
}

enum dfu_status sub_sm_process_dnload(struct fifo &dfu_fifo, enum dnload_sub_state &sub_state_arg)
{
  uint8_t page[DFU_FLASH_PAGE_SIZE_BYTES];
  
  switch (sub_state_arg) {
    case DNLOAD_SYNC:
      if (!flash_is_connected()) {
        t_profiler :> t_profiler_start;
        if (flash_init() != DFU_FLASH_OK) {
          // response = error_condition(DFU_errTARGET, 0);
          return DFU_errWRITE;
        }
        t_profiler :> t_profiler_end;
        t_profile_connect = t_profiler_end - t_profiler_start;
      }

      t_profiler :> t_profiler_start;
      // TODO - replace FLASH_MAX_UPGRADE_SIZE with image size from first page downloaded
      enum flash_status erase_status = flash_erase_sector_async(FLASH_MAX_UPGRADE_SIZE);
      if (erase_status != DFU_FLASH_OK && erase_status != DFU_FLASH_BUSY) {
        return DFU_errERASE;
      }
      t_profiler :> t_profiler_end;
      t_profile_first_erase = t_profiler_end - t_profiler_start;

      sub_transition_dnload(DNLOAD_ERASING, sub_state_arg);

      break;

    case DNLOAD_ERASING:
      // TODO - replace FLASH_MAX_UPGRADE_SIZE with image size from first page downloaded
      enum flash_status erase_status = flash_erase_sector_async(FLASH_MAX_UPGRADE_SIZE);

      if (erase_status == DFU_FLASH_OK) {
        // sector erase completed, move on to page write
        sub_transition_dnload(DNLOAD_WRITING, sub_state_arg);

        int32_t page_size_bytes = flash_get_page_size();
        if (fifo_block_dequeue(dfu_fifo, page, page_size_bytes) == FIFO_OK) {
          t_profiler :> t_profiler_start;
          if (flash_write_page(page, page_size_bytes) != DFU_FLASH_OK) {
            return DFU_errWRITE;
          }
          t_profiler :> t_profiler_end;
          t_profile_first_write = t_profiler_end - t_profiler_start;
        }

      } else if (erase_status == DFU_FLASH_BUSY) {
        // still erasing, remain in this state and wait for next poll
      } else {
        return DFU_errERASE;
      }
      break;

    case DNLOAD_WRITING:
      int32_t page_size_bytes = flash_get_page_size();
      if (fifo_block_dequeue(dfu_fifo, page, page_size_bytes) == FIFO_OK) {
        if (flash_write_page(page, page_size_bytes) != DFU_FLASH_OK) {
          return DFU_errWRITE;
        }
      }
      break;

      default:
        sub_state_arg = DNLOAD_SYNC;
        return DFU_errUNKNOWN;
  }

  return DFU_OK;
}

enum dfu_status sub_sm_process_manifest(struct fifo &dfu_fifo)
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

void sub_sm_print_profiler(void) {
  debug_printf("DFU: profile results:\n");
  debug_printf("  Connect time: %u ms\n", t_profile_connect);
  debug_printf("  First erase time: %u ms\n", t_profile_first_erase);
  debug_printf("  First write time: %u ms\n", t_profile_first_write);
}