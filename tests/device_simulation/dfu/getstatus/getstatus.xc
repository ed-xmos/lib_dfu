// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
#include <print.h>

#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

#include "dfu.h"

int main(void)
{
  struct dfu_getstatus ret = dfu_getstatus();
  assert(ret.status == DFU_OK);

  printstr("PASS\n");
  return 0;
}
