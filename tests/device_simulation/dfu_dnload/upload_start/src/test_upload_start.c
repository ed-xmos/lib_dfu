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

#define FIRST_READ_BYTE 0xAB
#define SECOND_READ_BYTE 0xCD

static int flash_open = 0;

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

struct flash_data_status flash_start_read() {
  struct flash_data_status result = { DFU_FLASH_OK, FLASH_MAX_UPGRADE_SIZE };
  return result;
}

enum flash_status flash_read_page(uint8_t* data, int32_t length) {
  (void)length;

  data[0] = FIRST_READ_BYTE;
  data[1] = SECOND_READ_BYTE;
  return DFU_FLASH_OK;
}

static uint8_t payload[DFU_TRANSFER_SIZE_BYTES];

static void get_state_and_check(enum dfu_state expected_state)
{
  struct dfu_cmd_response response = dfu_handle_read_command(DFU_GETSTATE, payload, DFU_GET_STATE_PAYLOAD_SIZE_BYTES);
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
  TEST_ASSERT_EQUAL(expected_state, payload[0]);
}

void test_upload_start(void) {
  struct dfu_getstatus getstatus;
  uint8_t block[DFU_TRANSFER_SIZE_BYTES] = { 0 };

  get_state_and_check(STATE_APP_IDLE);

  dfu_detach();
  get_state_and_check(STATE_APP_DETACH);

  dfu_bus_reset();
  get_state_and_check(STATE_DFU_IDLE);

  TEST_ASSERT_EQUAL_INT(0, flash_open);

  int read_size = dfu_upload(sizeof(block), block);
  TEST_ASSERT_EQUAL_INT(DFU_TRANSFER_SIZE_BYTES, read_size);

  get_state_and_check(STATE_DFU_UPLOAD_IDLE);

  getstatus = dfu_getstatus();
  TEST_ASSERT_EQUAL_INT(DFU_OK, getstatus.status);

  TEST_ASSERT_EQUAL_INT(1, flash_open);
  TEST_ASSERT_EQUAL_HEX8(FIRST_READ_BYTE, block[0]);
  TEST_ASSERT_EQUAL_HEX8(SECOND_READ_BYTE, block[1]);
}
