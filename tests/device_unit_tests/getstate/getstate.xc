// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <print.h>
#include <quadflash.h>

#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

#include "dfu.h"

int fl_getFactoryImage(fl_BootImageInfo &bootImageInfo)
{
  assert(0); // call not expected
  return 1;
}

int fl_getNextBootImage(fl_BootImageInfo &bootImageInfo)
{
  assert(0); // call not expected
  return 1;
}

int main(void)
{
  enum dfu_state state;

  state = dfu_getstate();
  assert(state == APP_IDLE);

  printstr("PASS\n");
  return 0;
}
