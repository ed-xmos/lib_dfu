// Copyright 2020-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <print.h>
#include <stdio.h>
#include <string.h>
#include <unity.h>
#include <xs1.h>

#define _Bool int
#include <stdbool.h>

#include "dfu.h"
#include "dfu_flash.h"


static int flash_open = 0;
static int erase_requested_size = 0;

enum flash_status flash_cmd_init() {
  flash_open = 1;
  return DFU_FLASH_OK;
}

enum flash_status flash_cmd_deinit() {
  flash_open = 0;
  return DFU_FLASH_OK;
}

int32_t flash_is_connected(void) {
  return flash_open;
}

struct flash_data_status flash_get_image_size_from_buffer(const uint8_t buf[], int32_t length) {
  (void)buf;
  (void)length;

  struct flash_data_status result = { DFU_FLASH_BAD_PARAM, 0 };
  return result;
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

struct flash_data_status flash_start_read() { 
  struct flash_data_status result = { DFU_FLASH_READ_ERROR, 0 };
  return result;
}

enum flash_status flash_read_page(uint8_t* data, int32_t length) {
  (void)data;
  (void)length;
  return DFU_FLASH_OK;
}

bool flash_is_busy(void) { return false; }

int32_t flash_get_page_size(void) { return 256; }

int32_t flash_get_sector_size(void) { return 4096; }

int32_t flash_get_size(void) { return 8 * 1024 * 1024; }

bool flash_is_suitable(void) { return true; }

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
  dfu_dnload(block_num, sizeof(block), block);

  state = dfu_getstate();
  TEST_ASSERT_EQUAL_INT(STATE_DFU_DNLOAD_SYNC, state);

  getstatus = dfu_getstatus();
  TEST_ASSERT_EQUAL_INT(ERR_OK, getstatus.status);

  TEST_ASSERT_EQUAL_INT(1, flash_open);
  TEST_ASSERT_EQUAL_INT(expected, erase_requested_size);
}
