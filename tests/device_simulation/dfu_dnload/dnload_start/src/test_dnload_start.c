// Copyright 2020-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <print.h>
#include <stdio.h>
#include <string.h>
#include <unity.h>
#include <xs1.h>

#include "dfu.h"
#include "dfu_flash.h"


static int flash_open = 0;
static int erase_requested_size = 0;

enum flash_status flash_init() {
  flash_open = 1;
  return DFU_FLASH_OK;
}

enum flash_status flash_deinit() {
  flash_open = 0;
  return DFU_FLASH_OK;
}

int32_t flash_is_connected(void) {
  return flash_open;
}

enum flash_status flash_erase_sector_async(int32_t erase_size) {
  erase_requested_size = erase_size;
  return DFU_FLASH_OK;
}

enum flash_status flash_write_page(const uint8_t page[], int32_t length) {
  (void)page;
  (void)length;
  return DFU_FLASH_OK;
}

enum flash_status flash_finalise_write() { return DFU_FLASH_OK; }

static uint8_t payload[DFU_TRANSFER_SIZE_BYTES];

static void get_state_and_check(enum dfu_state expected_state)
{
  struct dfu_cmd_response response = dfu_request_with_arguments(DFU_GETSTATE, payload, DFU_GET_STATE_PAYLOAD_SIZE_BYTES, NULL);
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
  TEST_ASSERT_EQUAL(expected_state, payload[0]);
}
static void get_status_and_check(enum dfu_status expected_status, enum dfu_state expected_state)
{
  struct dfu_cmd_response response = dfu_request_with_arguments(DFU_GETSTATUS, payload, DFU_GET_STATUS_PAYLOAD_SIZE_BYTES, NULL);
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
  TEST_ASSERT_EQUAL_UINT8(expected_status, payload[DFU_GETSTATUS_STATUS_INDEX]);
  TEST_ASSERT_EQUAL_UINT8(expected_state, payload[DFU_GETSTATUS_STATE_INDEX]);
}


void test_dnload_start(void) {
  uint8_t block[DFU_TRANSFER_SIZE_BYTES];
  // TODO - this should migrate to image size from first page downloaded,
  // but for now just check the expected value is passed to flash_erase_sector_async
  int32_t expected = FLASH_MAX_UPGRADE_SIZE;

  get_state_and_check(STATE_APP_IDLE);

  dfu_detach();
  get_state_and_check(STATE_APP_DETACH);

  dfu_bus_reset();
  get_state_and_check(STATE_DFU_IDLE);

  TEST_ASSERT_EQUAL_INT(0, flash_open);

  int32_t block_num = 0;
  struct dfu_cmd_response response = dfu_request_with_arguments(DFU_DNLOAD, block, sizeof(block), &block_num);
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);

  get_state_and_check(STATE_DFU_DOWNLOAD_SYNC);
  get_status_and_check(DFU_OK, STATE_DFU_DOWNLOAD_IDLE);

  /* flash is still not open as we now wait for a full page before flash access, as we need get-status request for deferred tasks */
  TEST_ASSERT_EQUAL_INT(0, flash_open);
  TEST_ASSERT_EQUAL_INT(0, erase_requested_size);
}
