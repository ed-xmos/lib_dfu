// Copyright 2019-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <unity.h>

#include "dfu.h"

static uint8_t payload[DFU_TRANSFER_SIZE_BYTES];

void test_getstate(void)
{
  struct dfu_cmd_response response = dfu_handle_read_command(DFU_GETSTATE, payload, DFU_GET_STATE_PAYLOAD_SIZE_BYTES);
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
  TEST_ASSERT_EQUAL(STATE_APP_IDLE, payload[0]);
}

void test_get_state_with_payload_size_too_big_reports_error(void)
{
  struct dfu_cmd_response response = dfu_handle_read_command(DFU_GETSTATE, payload, sizeof(payload));
  TEST_ASSERT_EQUAL(DFU_API_BAD_PARAM, response.status);
}
