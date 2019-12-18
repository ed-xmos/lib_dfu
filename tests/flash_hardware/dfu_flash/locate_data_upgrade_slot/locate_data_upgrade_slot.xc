// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <platform.h>
#include <stdio.h>
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

int main(unsigned argc, char * unsafe argv[argc])
{
  int ret;
  unsigned address = -1;
  unsigned data_partition_base = 0;

  assert(argc == 2);
  sscanf(argv[1], "%d", &data_partition_base);

  ret = flash_connect(ports, spec);
  assert(ret == 0);

  ret = flash_locate_data_upgrade_slot(address);
  assert(ret == 0);
  assert(address % spec[0].sectorSizes.regularSectorSize == 0);

  printintln(address);
  unsigned expected = data_partition_base + 4096 * 2;
  assert(address == expected);

  ret = flash_disconnect();
  assert(ret == 0);

  printstr("PASS\n");
  return 0;
}
