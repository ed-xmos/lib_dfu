// Copyright 2020-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <unity.h>

#include "dfu.h"

static uint8_t payload[DFU_TRANSFER_SIZE_BYTES];

static void get_state_and_check(enum dfu_state expected_state)
{
  struct dfu_cmd_response response = dfu_handle_read_command(DFU_GETSTATE, payload, DFU_GET_STATE_PAYLOAD_SIZE_BYTES);
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
  TEST_ASSERT_EQUAL(expected_state, payload[0]);
}

void test_bus_reset(void)
{
  get_state_and_check(STATE_APP_IDLE);

  struct dfu_cmd_response response = dfu_handle_write_command(DFU_DETACH, 0, NULL, 0);
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
  get_state_and_check(STATE_APP_DETACH);

  response = dfu_handle_write_command(XMOS_BUS_RESET, 0, NULL, 0);
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
  get_state_and_check(STATE_DFU_IDLE);

  response = dfu_handle_write_command(XMOS_BUS_RESET, 0, NULL, 0);
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
  get_state_and_check(STATE_APP_IDLE);
}
