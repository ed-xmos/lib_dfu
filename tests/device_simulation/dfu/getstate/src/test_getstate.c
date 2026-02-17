// Copyright 2019-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <unity.h>

#include "dfu.h"

void test_getstate(void)
{
  enum dfu_state state;

  state = dfu_getstate();
  TEST_ASSERT_EQUAL(STATE_APP_IDLE, state);
}
