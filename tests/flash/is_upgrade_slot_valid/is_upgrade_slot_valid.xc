// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <platform.h>
#include <print.h>

#define _Bool int
#include <stdbool.h>

#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

#include "dfu_flash.h"

fl_QSPIPorts ports = {
  PORT_SQI_CS, PORT_SQI_SCLK, PORT_SQI_SIO, XS1_CLKBLK_1
};

fl_QuadDeviceSpec spec[] = { // IS25LQ016B
  { 0, 256, 8192, 3, 8, 0x9F, 0, 3, 0x9D4015, 0x20, 4096, 0x06, 0x04,
    PROT_TYPE_NONE, {{0,0},{0x00,0x00}}, 0x02, 0xEB, 1,
    SECTOR_LAYOUT_REGULAR, {4096,{0,{0}}}, 0x05, 0x01, 0x01
  }
};

int main(unsigned argc, char * unsafe argv[argc])
{
  int ret;
  bool valid = false;
  bool expected;

  assert(argc == 2);

  ret = flash_connect(ports, spec);
  assert(ret == 0);

  ret = flash_is_upgrade_slot_valid(valid);
  assert(ret == 0);

  ret = flash_disconnect();
  assert(ret == 0);

  unsafe {
    expected = (argv[1][0] == 'V');
  }

  if (expected != valid) {
    printstr("expected ");
    printintln(expected);
    printstr("actual ");
    printintln(valid);
    return 1;
  }

  printstr("PASS\n");
  return 0;
}
