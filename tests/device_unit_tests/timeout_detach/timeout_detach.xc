// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <print.h>
#include "xassert.h"
#include "dfu.h"

int fl_connectToDevice(fl_QSPIPorts &ports, const fl_QuadDeviceSpec specs[], unsigned n)
{
  assert(0);
  return -1;
}

int main(void)
{
  enum dfu_state state;

  state = dfu_getstate();
  assert(state == APP_IDLE);

  dfu_detach();
  state = dfu_getstate();
  assert(state == APP_DETACH);

  dfu_timeout_detach();
  state = dfu_getstate();
  assert(state == APP_IDLE);

  printstr("PASS\n");
  return 0;
}
