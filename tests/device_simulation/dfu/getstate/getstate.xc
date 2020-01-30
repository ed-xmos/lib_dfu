// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <print.h>

#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

#include "dfu.h"

int main(void)
{
  enum dfu_state state;

  state = dfu_getstate();
  assert(state == APP_IDLE);

  printstr("PASS\n");
  return 0;
}
