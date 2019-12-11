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
  bool erased;

  ret = flash_connect(ports, spec);
  assert(ret == 0);

  // flash pre-loaded as:
  //    xflash --factory a.xe
  // or:
  //    xflash --erase-all --target=XCORE-200-EXPLORER

  erased = flash_is_sector_erased(0);

  ret = flash_disconnect();
  assert(ret == 0);

  if (erased)
    printstr("erased\n");
  else
    printstr("not erased\n");

  return 0;
}
