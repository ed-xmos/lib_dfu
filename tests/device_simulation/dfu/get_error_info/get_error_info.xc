// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <print.h>

#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

#include "dfu.h"

#define DFU_DETACH 0 // extracted from library implementation

int main(void)
{
  enum dfu_state state;
  struct dfu_slots slots = {4096, 8192};

  state = dfu_getstate();
  assert(state == APP_IDLE);

  dfu_detach();
  state = dfu_getstate();
  assert(state == APP_DETACH);

  dfu_bus_reset(slots);
  state = dfu_getstate();
  assert(state == DFU_IDLE);

  // another detach is unexpected here
  dfu_detach();
  state = dfu_getstate();
  assert(state == DFU_ERROR);

  int extra = dfu_get_error_info();
  assert(extra == DFU_DETACH);

  printstr("PASS\n");
  return 0;
}
