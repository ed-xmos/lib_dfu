// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <print.h>
#include <quadflash.h>

#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

#include "dfu.h"

int main(void)
{
  struct dfu_getstatus ret;

  ret = dfu_getstatus();
  assert(ret.state == APP_IDLE);
  assert(ret.status == DFU_OK);

  dfu_detach();
  ret = dfu_getstatus();
  assert(ret.state == APP_DETACH);
  assert(ret.status == DFU_OK);

  dfu_timeout_detach();
  ret = dfu_getstatus();
  assert(ret.state == APP_IDLE);
  assert(ret.status == DFU_OK);

  printstr("PASS\n");
  return 0;
}
