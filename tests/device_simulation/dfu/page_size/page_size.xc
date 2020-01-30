// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <platform.h>
#include <stdio.h>
#include <print.h>
#include <string.h>
#include <quadflash.h>

#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

#define DEBUG_UNIT TEST
#define DEBUG_PRINT_ENABLE_TEST 0
#include "debug_print.h"

#include "flash_data_partition.h"
#include "dfu.h"

fl_QSPIPorts ports = {
  PORT_SQI_CS, PORT_SQI_SCLK, PORT_SQI_SIO, XS1_CLKBLK_1
};

fl_QuadDeviceSpec spec[] = { // IS25LQ016B
  { 0, 256, 8192, 3, 8, 0x9F, 0, 3, 0x9D4015, 0x20, 4096, 0x06, 0x04,
    PROT_TYPE_NONE, {{0,0},{0x00,0x00}}, 0x02, 0xEB, 1,
    SECTOR_LAYOUT_REGULAR, {4096,{0,{0}}}, 0x05, 0x01, 0x01
  }
};

int flash_copy_specification(fl_QuadDeviceSpec copy[1])
{
  copy[0] = spec[0];
  return 0;
}

bool flash_is_first_whole_page_in_sector(unsigned address)
{
  return true;
}

int flash_erase_sector_async(unsigned address)
{
  return 0;
}

bool flash_is_busy(void)
{
  return false;
}

int main(unsigned argc, char * unsafe argv[argc])
{
  struct dfu_getstatus ret;
  enum dfu_state state;
  struct dfu_slots slots = {4096, 8192};
  int good = -1;
  char block[DFU_BLOCK_SIZE_MAX_BYTES];

  assert(argc == 3);

  unsafe {
    sscanf(argv[1], "%d", &spec[0].pageSize);
    sscanf(argv[2], "%d", &good);
  }

  state = dfu_getstate();
  assert(state == APP_IDLE);

  dfu_detach();
  state = dfu_getstate();
  assert(state == APP_DETACH);

  dfu_bus_reset(slots);
  state = dfu_getstate();
  assert(state == DFU_IDLE);

  dfu_dnload(0, sizeof(block), block);
  
  if (good) {
    state = dfu_getstate();
    assert(state == DFU_DNLOAD_SYNC);

    ret = dfu_getstatus();
    assert(ret.status == DFU_OK);
  }
  else {
    state = dfu_getstate();
    assert(state == DFU_ERROR);

    ret = dfu_getstatus();
    assert(ret.status == ERR_UNKNOWN);
  }

  printstr("PASS\n");
  return 0;
}
