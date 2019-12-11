// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <platform.h>
#include <print.h>

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

int main(void)
{
  int ret;
  unsigned address = -1;
  int timeout = 1000;
  bool valid = false;
  int start, end;
  timer tmr;

  ret = flash_connect(ports, spec);
  assert(ret == 0);

  // flash pre-loaded as: xflash --factory a.xe --upgrade 1 a.xe

  ret = flash_is_upgrade_slot_valid(valid);
  assert(ret == 0);
  assert(valid);

  ret = flash_locate_upgrade_slot(address);
  assert(ret == 0);

  ret = flash_erase_sector_async(address);
  assert(ret == 0);
  tmr :> start;

  while (flash_is_busy() && timeout > 0) {
    delay_milliseconds(1);
    timeout--;
  }

  tmr :> end;
  assert(timeout > 0);
  printintln((end - start) / 100000); // erase duration in msec

  ret = flash_is_upgrade_slot_valid(valid);
  assert(ret == 0);
  assert(!valid);

  flash_disconnect();
  assert(ret == 0);

  printstr("PASS\n");
  return 0;
}
