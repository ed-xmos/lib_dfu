// Copyright 2019-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <unity.h>

#include "dfu.h"

void test_getstatus(void)
{
  struct dfu_getstatus ret = dfu_getstatus();
  TEST_ASSERT_EQUAL(ERR_OK, ret.status);
}
