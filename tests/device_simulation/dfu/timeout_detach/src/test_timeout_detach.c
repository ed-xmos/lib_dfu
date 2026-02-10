// Copyright 2019-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <unity.h>

#include "dfu.h"

void test_timeout_detach(void)
{
  struct dfu_getstatus ret;

  ret = dfu_getstatus();
  TEST_ASSERT_EQUAL(STATE_APP_IDLE, ret.state);
  TEST_ASSERT_EQUAL(ERR_OK, ret.status);

  dfu_detach();
  ret = dfu_getstatus();
  TEST_ASSERT_EQUAL(STATE_APP_DETACH, ret.state);
  TEST_ASSERT_EQUAL(ERR_OK, ret.status);

  dfu_timeout_detach();
  ret = dfu_getstatus();
  TEST_ASSERT_EQUAL(STATE_APP_IDLE, ret.state);
  TEST_ASSERT_EQUAL(ERR_OK, ret.status);
}
