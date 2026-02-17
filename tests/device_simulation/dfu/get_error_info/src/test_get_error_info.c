// Copyright 2020-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <unity.h>

#include "dfu.h"

void test_get_error_info(void)
{
  enum dfu_state state;

  state = dfu_getstate();
  TEST_ASSERT_EQUAL(STATE_APP_IDLE, state);

  dfu_detach();
  state = dfu_getstate();
  TEST_ASSERT_EQUAL(STATE_APP_DETACH, state);

  dfu_bus_reset();
  state = dfu_getstate();
  TEST_ASSERT_EQUAL(STATE_DFU_IDLE, state);

  // another detach is unexpected here
  dfu_detach();
  state = dfu_getstate();
  TEST_ASSERT_EQUAL(STATE_DFU_ERROR, state);

  int extra = dfu_get_error_info();
  TEST_ASSERT_EQUAL(DFU_DETACH, extra); // TODO - check this...
}
