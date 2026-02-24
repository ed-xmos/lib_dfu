// Copyright 2019-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <xs1.h>
#include <platform.h>
#include <stdio.h>
#include <stddef.h>
#include <print.h>
#include <string.h>
#include <quadflash.h>
// #include <quadflashlib.h>

#include <unity.h>

#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

#include "dfu.h"

int transfer_size_bytes;
FILE* dfu_file;

void setUp(void) {}
void tearDown(void) {}

static uint8_t payload[DFU_TRANSFER_SIZE_BYTES];

static void get_state_and_check(enum dfu_state expected_state)
{
  struct dfu_cmd_response response = dfu_handle_read_command(DFU_GETSTATE, payload, DFU_GET_STATE_PAYLOAD_SIZE_BYTES);
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
  TEST_ASSERT_EQUAL(expected_state, payload[0]);
}

void detach()
{
  get_state_and_check(STATE_APP_IDLE);

  dfu_detach();
  get_state_and_check(STATE_APP_DETACH);

  dfu_bus_reset();
  get_state_and_check(STATE_DFU_IDLE);
}

FILE * write(FILE * bin_file, int block_size, int *upgrade_size)
{
  struct dfu_getstatus ret;
  int block_count = 0;
  size_t read;
  uint8_t block[DFU_TRANSFER_SIZE_BYTES];

  while (!feof(bin_file)) {
    printintln(block_count);

    read = fread(block, 1, (size_t)block_size, bin_file);
    assert(read <= (size_t)block_size);

    if (read == 0)
      break;

    dfu_dnload(block_count, (int32_t)read, block);

    do {
      ret = dfu_getstatus();
      assert(ret.status == DFU_OK);
      delay_microseconds(1);
    } while (ret.state == STATE_DFU_DOWNLOAD_BUSY);

    assert(ret.state == STATE_DFU_DOWNLOAD_IDLE);

    block_count++;
  }

  dfu_dnload(0, 0, block);
  get_state_and_check(STATE_DFU_MANIFEST_SYNC);

  ret = dfu_getstatus();
  assert(ret.state == STATE_DFU_IDLE);
  assert(ret.status == DFU_OK);

  *upgrade_size = block_count * block_size;

  return bin_file;
}

void printhexbufferln(uint8_t x[], int width)
{
  for( int i = 0; i < width; i++) {
    printf("%02x ", x[i]);
  }
  printf("\n");
  return;
}

FILE * verify(FILE * bin_file, int block_size)
{
  int page_count = 0;
  uint8_t expected[256], actual[256];
  (void) block_size;

  assert(block_size <= (int)sizeof(expected));

  while (!feof(bin_file)) {
    printintln(page_count);

    size_t ret = fread(expected, 1, (size_t)block_size, bin_file);
    assert(ret <= (size_t)block_size);

    if (ret == 0)
      break;

    struct dfu_cmd_response response = dfu_handle_read_command(DFU_UPLOAD, actual, (size_t)block_size);
    TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
    TEST_ASSERT_LESS_OR_EQUAL_INT32(block_size, response.return_data_len);

    int read_size = response.return_data_len;
    assert(read_size <= (int)ret);
    if (read_size < (int)ret) {
      printf("Short read from device: pages %d, read %d, total %d\n", page_count, read_size, (page_count * block_size) + read_size);
    }

    int match = memcmp(expected, actual, (size_t)read_size);
    if (match != 0) {
      printf("Verification failed at page %d\n", page_count);
      printhexbufferln(expected, block_size);
      printhexbufferln(actual, block_size);
    }
    assert(match == 0);

    page_count++;
    // Short read is EOF on Flash
    if (read_size < (int)block_size) {
      break;
    }
  }

  return bin_file;
}

void test_write_upgrade_boot_only()
{
  // Block size likely supports up to Flash page size operations, test.
  int block_size = transfer_size_bytes;

  assert(block_size <= DFU_TRANSFER_SIZE_BYTES);
  printf("Writing block size %d\n", block_size);

  detach();

  int upgrade_size = 0;
  dfu_file = write(dfu_file, block_size, &upgrade_size);
  printf("written %d bytes\n", upgrade_size);

  fseek(dfu_file, 0, SEEK_SET);

  dfu_file = verify(dfu_file, block_size);
  printf("verified\n");
}
