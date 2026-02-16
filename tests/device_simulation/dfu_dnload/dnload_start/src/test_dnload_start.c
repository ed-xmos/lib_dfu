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

void test_dnload_start(void) {
  struct dfu_getstatus getstatus;
  enum dfu_state state;
  uint8_t block[DFU_TRANSFER_SIZE_BYTES];
  // TODO - this should migrate to image size from first page downloaded,
  // but for now just check the expected value is passed to flash_erase_sector_async
  int32_t expected = FLASH_MAX_UPGRADE_SIZE;

  state = dfu_getstate();
  TEST_ASSERT_EQUAL_INT(STATE_APP_IDLE, state);

  dfu_detach();
  state = dfu_getstate();
  TEST_ASSERT_EQUAL_INT(STATE_APP_DETACH, state);

  dfu_bus_reset();
  state = dfu_getstate();
  TEST_ASSERT_EQUAL_INT(STATE_DFU_IDLE, state);

  TEST_ASSERT_EQUAL_INT(0, flash_open);

  int block_num = 0;
  struct dfu_cmd_response response = dfu_handle_write_command(DFU_DNLOAD, block_num, block, sizeof(block));
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);

  state = dfu_getstate();
  TEST_ASSERT_EQUAL_INT(STATE_DFU_DOWNLOAD_SYNC, state);

  getstatus = dfu_getstatus();
  TEST_ASSERT_EQUAL_INT(DFU_OK, getstatus.status);

  TEST_ASSERT_EQUAL_INT(1, flash_open);
  TEST_ASSERT_EQUAL_INT(expected, erase_requested_size);
}
