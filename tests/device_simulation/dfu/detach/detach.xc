// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
#include <print.h>

#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

#include "dfu.h"

int main(void)
{
  struct dfu_getstatus ret;
  enum dfu_state state;
  struct dfu_slots slots = {4096, 8192};

  state = dfu_getstate();
  assert(state == APP_IDLE);

  dfu_detach();
  ret = dfu_getstatus();
  assert(ret.state == APP_DETACH);
  assert(ret.status == DFU_OK);

  dfu_bus_reset(slots);
  ret = dfu_getstatus();
  assert(ret.state == DFU_IDLE);
  assert(ret.status == DFU_OK);

  printstr("PASS\n");
  return 0;
}
