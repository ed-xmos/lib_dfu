// Copyright 2019-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <unity.h>

#include "dfu.h"

static uint8_t payload[DFU_TRANSFER_SIZE_BYTES];

static void get_status_and_check(enum dfu_status expected_status, enum dfu_state expected_state)
{
  struct dfu_cmd_response response = dfu_request_with_arguments(DFU_GETSTATUS, payload, DFU_GET_STATUS_PAYLOAD_SIZE_BYTES, NULL);
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
  TEST_ASSERT_EQUAL_UINT8(expected_status, payload[DFU_GETSTATUS_STATUS_INDEX]);
  TEST_ASSERT_EQUAL_UINT8(expected_state, payload[DFU_GETSTATUS_STATE_INDEX]);
}

void test_timeout_detach(void)
{
  get_status_and_check(DFU_OK, STATE_APP_IDLE);

  dfu_request(DFU_DETACH);
  get_status_and_check(DFU_OK, STATE_APP_DETACH);

  dfu_timeout_detach();
  get_status_and_check(DFU_OK, STATE_APP_IDLE);
}
