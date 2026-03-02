// Copyright 2020-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <unity.h>

#include "dfu.h"

static uint8_t payload[DFU_TRANSFER_SIZE_BYTES];

static void get_state_and_check(enum dfu_state expected_state)
{
  struct dfu_cmd_response response = dfu_request_with_arguments(DFU_GETSTATE, payload, DFU_GET_STATE_PAYLOAD_SIZE_BYTES, NULL);
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
  TEST_ASSERT_EQUAL(expected_state, payload[DFU_GETSTATE_INDEX]);
}

void test_clrstatus(void)
{
  enum dfu_state state;

  get_state_and_check(STATE_APP_IDLE);

  struct dfu_cmd_response response = dfu_request(DFU_DETACH);
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
  get_state_and_check(STATE_APP_DETACH);

  response = dfu_request(XMOS_DFU_BUS_RESET);
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
  get_state_and_check(STATE_DFU_IDLE);

  // another detach is unexpected here
  response = dfu_request(DFU_DETACH);
  TEST_ASSERT_EQUAL(DFU_API_ERROR, response.status);
  get_state_and_check(STATE_DFU_ERROR);

  // clear error state
  response = dfu_request(DFU_CLRSTATUS);
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
  get_state_and_check(STATE_DFU_IDLE);
}
