// Copyright 2019-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <print.h>
#include <stdio.h>
#include <string.h>
#include <unity.h>
#include <xclib.h>
#include <xs1.h>

#define _Bool int
#include <stdbool.h>

#define DEBUG_UNIT TEST
#define DEBUG_PRINT_ENABLE_TEST 1
#include "debug_print.h"
#include "dfu.h"
#include "dfu_flash.h"

// See lib.xc for crc32 implementation
void crc32_c(unsigned *checksum, unsigned data, unsigned poly);

#define MAX_IMAGE_SIZE 20480

struct {
  int state_reading;
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

struct flash_data_status flash_get_image_size_from_buffer(const uint8_t buf[], int32_t length) {
  (void)buf;
  (void)length;

  struct flash_data_status result = { DFU_FLASH_BAD_PARAM, fl.partitions.u_size };
  return result;
}

struct flash_data_status flash_start_read() {
  fl.state_reading = 1;
  fl.address = fl.partitions.u_start;

  struct flash_data_status result = { DFU_FLASH_OK, fl.partitions.u_size };
  return result;
}

enum flash_status flash_read_page(uint8_t *data, int32_t length) {
  if (length != 256 || data == NULL) {
    return DFU_FLASH_BAD_PARAM;

  } else if (!fl.state_reading) {
    return DFU_FLASH_READ_ERROR;
  }

  if (fl.address >= fl.partitions.u_start && fl.address < fl.partitions.u_start + fl.partitions.u_size) {
    int contents_offset = fl.address - fl.partitions.u_start;
    debug_printf("read %s upgrade offset 0x%X (flash page %d)\n", labels, contents_offset, fl.address / 256);

    memcpy(data, &fl.partitions.u_contents[contents_offset], 256);
    fl.address += 256;
    if (fl.address >= fl.partitions.u_start + fl.partitions.u_size) {
      fl.state_reading = 0;
    }
    return DFU_FLASH_OK;

  } else {
    return DFU_FLASH_READ_NO_IMAGE;
  }
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

void make_test_data(char seq[], int length) {
  // memset(seq, 33, (size_t)length);
  for (int i = 0; i < length; i++) {
    unsigned x;
    crc32_c(&x, (unsigned)-1, 0xEB31D82EU);
    seq[i] = (char)x;
  }
}

void layout_flash(int block_count, int block_size, int tail_size) {
  debug_printf("image blocks %d x %d bytes + %d bytes tail\n", block_count, block_size, tail_size);

  fl.state_reading = 0;
  fl.address = 0;
  fl.partitions.base = 0;
  fl.flash_open = 0;

  fl.partitions.f_start = fl.partitions.base + 4096;
  fl.partitions.f_size = 256;
  fl.partitions.u_start = fl.partitions.f_start + 4096;
  fl.partitions.u_size = block_count * block_size + tail_size;
  make_test_data(fl.partitions.u_contents, fl.partitions.u_size);

  debug_printf("%s partition: factory 0x%X (%d), upgrade 0x%X (%d)\n", labels, fl.partitions.f_start,
               fl.partitions.f_size, fl.partitions.u_start, fl.partitions.u_size);

  fl.busy_countdown = 0;
}

static uint8_t payload[DFU_TRANSFER_SIZE_BYTES];

static void get_state_and_check(enum dfu_state expected_state)
{
  struct dfu_cmd_response response = dfu_handle_read_command(DFU_GETSTATE, payload, DFU_GET_STATE_PAYLOAD_SIZE_BYTES);
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
  TEST_ASSERT_EQUAL(expected_state, payload[0]);
}

void detach() {
  // TODO pass if we are already in DFU_IDLE from previous test.

  get_state_and_check(STATE_APP_IDLE);

  dfu_detach();
  get_state_and_check(STATE_APP_DETACH);

  dfu_bus_reset();
  get_state_and_check(STATE_DFU_IDLE);
}

void reboot() {
  get_state_and_check(STATE_DFU_IDLE);

  dfu_bus_reset();
  get_state_and_check(STATE_APP_IDLE);
}

void upload(unsigned char images[MAX_IMAGE_SIZE], int block_size, int block_count, int tail_size) {
  const int marker = 0;  // Was, DFU_BLOCK_NUM_DATA_IMAGE_MARKER * p;
  
  for (int i = 0; i < block_count; i++) {
    debug_printf("upload block %d 0x%04X (%d bytes)\n", i, marker | i, block_size);

    struct dfu_cmd_response response = dfu_handle_read_command(DFU_UPLOAD, &images[i * block_size], (size_t)block_size);
    TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
    TEST_ASSERT_EQUAL(block_size, response.return_data_len);
    
    struct dfu_getstatus status = dfu_getstatus();
    TEST_ASSERT_EQUAL(DFU_OK, status.status);
    TEST_ASSERT_EQUAL(STATE_DFU_UPLOAD_IDLE, status.state);
  }
  if (tail_size >= 0) {
    debug_printf("upload block %d 0x%04X (tail %d bytes)\n", block_count, marker | block_count, tail_size);

    struct dfu_cmd_response response = dfu_handle_read_command(DFU_UPLOAD, &images[block_count * block_size], (size_t)block_size);
    TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
    TEST_ASSERT_EQUAL(tail_size, response.return_data_len);
  }
  get_state_and_check(STATE_DFU_IDLE);

  /* Sanity checks */
  TEST_ASSERT_FALSE(fl.state_reading);
  TEST_ASSERT_FALSE(fl.flash_open);
}

void verify(const uint8_t images[MAX_IMAGE_SIZE]) {
  debug_printf("verify\n");

  int cmp = memcmp(fl.partitions.u_contents, images, (size_t)fl.partitions.u_size);
  TEST_ASSERT_EQUAL(0, cmp);
}

static uint8_t images[MAX_IMAGE_SIZE];

void test_upload_no_tail(void) {
  int block_size = 0;
  int block_count = 0;
  int tail_size = 0;
  int repeats = 0;

  block_size = 64;  // bytes
  block_count = 6;  // blocks
  tail_size = 0;   // bytes, ideally less than block_size
  repeats = 2;

  layout_flash(block_count, block_size, tail_size);

  detach();
  
  for (int r = 0; r < repeats; r++) {
    upload(images, block_size, block_count, tail_size);

    verify(images);
    
    memset(images, 0, (size_t)(MAX_IMAGE_SIZE));
  }

  reboot();
}

void test_upload_with_tail(void) {
  int block_size = 0;
  int block_count = 0;
  int tail_size = 0;
  int repeats = 0;

  block_size = 64;  // bytes
  block_count = 6;  // blocks
  tail_size = 5;   // bytes, ideally less than block_size
  repeats = 2;

  layout_flash(block_count, block_size, tail_size);

  detach();
  
  for (int r = 0; r < repeats; r++) {
    upload(images, block_size, block_count, tail_size);

    verify(images);
    
    memset(images, 0, (size_t)(MAX_IMAGE_SIZE));
  }
  
  reboot();
}
