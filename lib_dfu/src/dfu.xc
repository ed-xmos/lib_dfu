// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include "dfu.h"

static enum dfu_state state = APP_IDLE;

enum dfu_state dfu_getstate(void)
{
  return state;
}
