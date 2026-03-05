// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef DFU_SUB_SM_H
#define DFU_SUB_SM_H

#include <stdint.h>

#include "dfu_types.h"
#include "fifo.h"

struct dfu_sub_request {
  enum dfu_request request;
  int32_t time_allowed_msec;
};

struct dfu_sub_response {
  enum dfu_status status;
  int32_t poll_timeout_msec;
};

enum dnload_sub_state {
  DNLOAD_SYNC,
  DNLOAD_ERASING,
  DNLOAD_WRITING
};

enum dfu_status sub_sm_process_dnload(struct fifo &dfu_fifo, enum dnload_sub_state &sub_state_arg);
enum dfu_status sub_sm_process_manifest(struct fifo &dfu_fifo);

void sub_sm_print_profiler(void);

#endif /* DFU_SUB_SM_H */
