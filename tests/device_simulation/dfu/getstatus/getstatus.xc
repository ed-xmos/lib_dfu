// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <print.h>
#include <quadflash.h>

#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

#include "dfu.h"

int main(void)
{
  enum dfu_status status;
  enum dfu_state state;
  unsigned timeout;

  {status, state, timeout} = dfu_getstatus();
  assert(status == DFU_OK);

  printstr("PASS\n");
  return 0;
}
