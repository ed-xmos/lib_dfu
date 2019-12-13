// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <platform.h>
#include <stdio.h>
#include <stddef.h>
#include <print.h>
#include <string.h>
#include <quadflash.h>

#define _Bool int
#include <stdbool.h>

#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

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

unsafe {
  fl_QSPIPorts * unsafe p_ports = (fl_QSPIPorts * unsafe)&ports;
  fl_QuadDeviceSpec * unsafe p_spec = (fl_QuadDeviceSpec * unsafe)&spec;
}

int main(unsigned argc, char * unsafe argv[argc])
{
  enum dfu_state state;
  enum dfu_status status;
  unsigned timeout;
  unsigned block_count = 0;
  size_t ret;
  char block[DFU_BLOCK_SIZE_MAX_BYTES];

  FILE * movable bin_file = fopen((char*)argv[1], "rb");

  state = dfu_getstate();
  assert(state == APP_IDLE);

  dfu_detach();
  state = dfu_getstate();
  assert(state == APP_DETACH);

  dfu_bus_reset(ports, spec);
  state = dfu_getstate();
  assert(state == DFU_IDLE);

  while (!feof(bin_file)) {
    printintln(block_count);

    ret = fread(block, 1, sizeof(block), bin_file);
    assert(ret >= 0 && ret <= sizeof(block));

    if (ret == 0)
      break;

    dfu_dnload(block_count, sizeof(block), block);

    do {
      {status, state, timeout} = dfu_getstatus();
      assert(status == DFU_OK);
      delay_microseconds(1);
    } while (state == DFU_DNBUSY);

    assert(state == DFU_DNLOAD_IDLE);

    block_count++;
  }

  dfu_dnload(0, 0, block);
  state = dfu_getstate();
  assert(state == DFU_MANIFEST_SYNC);

  {status, state, timeout} = dfu_getstatus();
  assert(state == DFU_IDLE);
  assert(status == DFU_OK);

  fclose(move(bin_file));

  printstr("PASS\n");
  return 0;
}
