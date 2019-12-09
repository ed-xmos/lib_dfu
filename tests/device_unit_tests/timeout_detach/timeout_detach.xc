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
  enum dfu_status status;
  unsigned timeout;

  {status, state, timeout} = dfu_getstatus();
  assert(state == APP_IDLE);
  assert(status == DFU_OK);

  dfu_detach();
  {status, state, timeout} = dfu_getstatus();
  assert(state == APP_DETACH);
  assert(status == DFU_OK);

  dfu_timeout_detach();
  {status, state, timeout} = dfu_getstatus();
  assert(state == APP_IDLE);
  assert(status == DFU_OK);

  printstr("PASS\n");
  return 0;
}
