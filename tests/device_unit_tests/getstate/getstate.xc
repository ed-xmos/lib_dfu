// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stdio.h>
#include "xassert.h"
#include "dfu.h"

int main(void)
{
  enum dfu_state state;

  state = dfu_getstate();
  assert(state == APP_IDLE);

  return 0;
}
