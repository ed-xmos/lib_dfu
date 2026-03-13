// Copyright 2019-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <print.h>
#include <stdio.h>
#include <string.h>
#include <unity.h>
#include <xclib.h>
#include <xs1.h>

#define DEBUG_UNIT TEST
#define DEBUG_PRINT_ENABLE_TEST 1
#include "debug_print.h"
#include "dfu.h"
#include "dfu_flash.h"

// See lib.xc for crc32 implementation
void crc32_c(unsigned *checksum, unsigned data, unsigned poly);

#define MAX_IMAGE_SIZE 20480

struct {
  int state_erasing;
  unsigned erase_size;
  int flash_open;
  int address;
  struct {
    int base;
    int f_start;
    int f_size;
    int u_start;
    int u_size;
    char u_contents[MAX_IMAGE_SIZE];
  } partitions;
  int busy_countdown;
  char page_erased[8192];    // use 8bit char instead of 32bit bool
} fl;

const char *labels = "boot";

enum flash_status flash_init() {
  fl.flash_open = 1;
  return DFU_FLASH_OK;
}

enum flash_status flash_deinit() {
  fl.flash_open = 0;
  return DFU_FLASH_OK;
}

int32_t flash_is_connected(void) { return fl.flash_open; }

enum flash_status flash_erase_sector_async(int32_t erase_size) {
  (void)erase_size;

  if (!fl.state_erasing) {
    fl.state_erasing = 1;
    /* Overriding erase size during test */
    fl.erase_size = MAX_IMAGE_SIZE;
    fl.address = fl.partitions.u_start;

  } else {
    if (fl.address >= fl.partitions.u_start + fl.partitions.u_size) {
      fl.state_erasing = 0;
      return DFU_FLASH_OK;
    }
  }
  debug_printf("flash_erase_sector_async 0x%X\n", fl.address);
  
  TEST_ASSERT_EQUAL(0, fl.busy_countdown);

  for (int i = 0; i < (4096 / 256); i++) {
    int page_address = fl.address + 256 * i;
    int page_index = fl.address / 256 + i;

    fl.page_erased[page_index] = 1;

    if (page_address >= fl.partitions.u_start &&
        page_address < fl.partitions.u_start + fl.partitions.u_size) {

      int contents_offset = fl.address - fl.partitions.u_start + 256 * i;
      debug_printf("erase %s upgrade offset 0x%X (flash page %d)\n",
                    labels, contents_offset, page_index);

      memset(&fl.partitions.u_contents[contents_offset], 0xFF, 256);
    }
  }
  fl.busy_countdown = 0;

  if (fl.state_erasing) {
    fl.address += 4096;
  }

  return DFU_FLASH_BUSY;
}

struct flash_data_status flash_get_image_size_from_buffer(const uint8_t buf[], int32_t length) {
  (void)buf;
  (void)length;

  struct flash_data_status result = { DFU_FLASH_BAD_PARAM, fl.partitions.u_size };
  return result;
}

int32_t flash_is_busy(void) {
  if (fl.busy_countdown > 0) {
    debug_printf("busy countdown %d\n", fl.busy_countdown);
    fl.busy_countdown--;
    return 1;
  } else {
    return 0;
  }
}

static uint8_t payload[DFU_TRANSFER_SIZE_BYTES];

static void get_state_and_check(enum dfu_state expected_state)
{
  struct dfu_cmd_response response = dfu_request_with_arguments(DFU_GETSTATE, payload, DFU_GET_STATE_PAYLOAD_SIZE_BYTES, NULL);
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
  TEST_ASSERT_EQUAL(expected_state, payload[0]);
}

static void bus_reset() {
  struct dfu_cmd_response response = dfu_request(XMOS_DFU_BUS_RESET);
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
  if (response.deferred_request == DFU_DEFERRED_ACTION_FLASH_CONNECT) {
    response = dfu_request(response.deferred_request);
    TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
  }
}

void detach() {
  // TODO pass if we are already in DFU_IDLE from previous test.

  get_state_and_check(STATE_APP_IDLE);

  dfu_detach();
  get_state_and_check(STATE_APP_DETACH);

  bus_reset();
  get_state_and_check(STATE_DFU_IDLE);
}

void reboot() {
  get_state_and_check(STATE_DFU_IDLE);

  bus_reset();
  get_state_and_check(STATE_APP_IDLE);
}

void revert() {
  get_state_and_check(STATE_DFU_IDLE);

  struct dfu_cmd_response response = dfu_request(XMOS_DFU_REVERTFACTORY);
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
}

void test_revert(void) {
  detach();
  
  TEST_ASSERT_TRUE(fl.flash_open);

  revert();

  /* Check we have erase the first sector of the update image. */
  TEST_ASSERT_TRUE(fl.page_erased[fl.partitions.u_start / 256]);

  reboot();

  TEST_ASSERT_FALSE(fl.flash_open);
}
