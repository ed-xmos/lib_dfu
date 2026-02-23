// Copyright 2019-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <unity.h>

#include "dfu.h"

void test_detach(void)
{
  struct dfu_getstatus ret;
  enum dfu_state state;

  state = dfu_getstate();
  TEST_ASSERT_EQUAL(STATE_APP_IDLE, state);

  dfu_detach();
  ret = dfu_getstatus();
  TEST_ASSERT_EQUAL(STATE_APP_DETACH, ret.state);
  TEST_ASSERT_EQUAL(DFU_OK, ret.status);

  dfu_bus_reset();
  ret = dfu_getstatus();
  TEST_ASSERT_EQUAL(STATE_DFU_IDLE, ret.state);
  TEST_ASSERT_EQUAL(DFU_OK, ret.status);
}
