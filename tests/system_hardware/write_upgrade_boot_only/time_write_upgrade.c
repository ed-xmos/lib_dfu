// Copyright 2019-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <xs1.h>
#include <platform.h>
#include <stdio.h>
#include <stddef.h>
#include <print.h>
#include <string.h>
#include <quadflash.h>
#include <quadflashlib.h>

#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

#define DEBUG_UNIT TEST
#define DEBUG_PRINT_ENABLE_TEST 0 // requires xSCOPE
#include "debug_print.h"

#include "quadflash_extra.h"
#include "dfu.h"

fl_QSPIPorts ports = {
  PORT_SQI_CS, PORT_SQI_SCLK, PORT_SQI_SIO, XS1_CLKBLK_1
};

fl_QuadDeviceSpec spec = { // IS25LQ016B
  0, 256, 8192, 3, 8, 0x9F, 0, 3, 0x9D4015, 0x20, 4096, 0x06, 0x04,
  PROT_TYPE_NONE, {{0,0},{0x00,0x00}}, 0x02, 0xEB, 1,
  SECTOR_LAYOUT_REGULAR, {4096,{0,{0}}}, 0x05, 0x01, 0x01
};

struct {
  int start;
  int locator;
  int threshold;
} timing = {0, 0, 0};

static void t_start(int locator) {
  timer t;
  timing.locator = locator;
  t :> timing.start;
}

static void t_end(void) {
  timer t;
  int start = timing.start;
  int end;
  t :> end;
  if (end - start >= timing.threshold) {
    debug_printf("%d: %d\n", timing.locator, end - start); // requires xSCOPE
    assert(0);
  }
}

static uint8_t payload[DFU_TRANSFER_SIZE_BYTES];

static enum dfu_state get_state()
{
  struct dfu_cmd_response response = dfu_request_with_arguments(DFU_GETSTATE, payload, DFU_GET_STATE_PAYLOAD_SIZE_BYTES, NULL);
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
  return payload[0];
}

static struct dfu_getstatus get_status()
{
  struct dfu_cmd_response response = dfu_request_with_arguments(DFU_GETSTATUS, payload, DFU_GET_STATUS_PAYLOAD_SIZE_BYTES, NULL);
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);

  struct dfu_getstatus ret = { .status = payload[DFU_GETSTATUS_STATUS_INDEX], .state = payload[DFU_GETSTATUS_STATE_INDEX] };
  return ret;
}

static void bus_reset() {
  struct dfu_cmd_response response = dfu_request(XMOS_DFU_BUS_RESET);
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
  if (response.deferred_request == DFU_DEFERRED_ACTION_FLASH_CONNECT) {
    response = dfu_request(response.deferred_request);
    TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
  }
}

void write_begin(void)
{
  enum dfu_state state;
  int ret;

  ret = fl_connectToOneDevice(ports, spec);
  assert(ret == 0);

  dfu_locate_upgrade_slots();
  fl_disconnect();

  t_start(1);
  state = get_state();
  t_end();
  assert(state == STATE_APP_IDLE);

  t_start(2);
  dfu_detach();
  t_end();
  t_start(3);
  state = get_state();
  t_end();
  assert(state == STATE_APP_DETACH);

  ret = fl_connectToOneDevice(ports, spec);
  assert(ret == 0);

  t_start(4);
  bus_reset();
  t_end();
  t_start(5);
  state = get_state();
  t_end();
  assert(state == STATE_DFU_IDLE);
}

FILE * movable write(FILE * movable bin_file, int block_size, int marker)
{
  struct dfu_getstatus ret;
  enum dfu_state state;
  size_t read;
  char block[DFU_TRANSFER_SIZE_BYTES];
  int block_count = 0;

  while (!feof(bin_file)) {
    printintln(block_count);

    read = fread(block, 1, block_size, bin_file);
    assert(read >= 0 && read <= block_size);

    if (read == 0)
      break;

    t_start(6);
    struct dfu_cmd_response response = dfu_request_with_arguments(DFU_DNLOAD, block, read, (marker | block_count));
    TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
    t_end();

    do {
      t_start(7);
      ret = get_status();
      t_end();
      assert(ret.status == DFU_OK);
      delay_milliseconds(ret.poll_timeout_msec);
    } while (ret.state == STATE_DFU_DNBUSY);

    assert(ret.state == STATE_DFU_DNLOAD_IDLE);

    block_count++;
  }

  t_start(8);block_count
  struct dfu_cmd_response response = dfu_request_with_arguments(DFU_DNLOAD, block, 0, NULL);
  TEST_ASSERT_EQUAL(DFU_API_SUCCESS, response.status);
  t_end();
  state = get_state();
  t_end();
  assert(state == STATE_DFU_MANIFEST_SYNC);

  do {
    t_start(9);
    ret = get_status();
    t_end();
    assert(ret.status == DFU_OK);
    // Short delay for testing purposes.
    delay_microseconds(ret.poll_timeout_msec);
  } while (ret.state == STATE_DFU_DOWNLOAD_BUSY);

  return move(bin_file);
}

int main(unsigned argc, char * unsafe argv[argc])
{
  const int block_size = 128;

  assert(argc == 4);

  FILE * movable boot_file = fopen((char*)argv[1], "rb");
  FILE * movable data_file = fopen((char*)argv[2], "rb");
  unsafe {
    sscanf(argv[3], "%d", &timing.threshold);
  }

  write_begin();

  boot_file = write(move(boot_file), block_size, 0);
  data_file = write(move(data_file), block_size, DFU_BLOCK_NUM_DATA_IMAGE_MARKER);

  fclose(move(boot_file));
  fclose(move(data_file));

  fl_disconnect();

  printstr("PASS\n");
  return 0;
}
