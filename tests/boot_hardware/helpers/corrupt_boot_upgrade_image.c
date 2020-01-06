// Copyright (c) 2020, XMOS Ltd, All rights reserved
//
// Build with quadflash library, eg:
//
// xcc corrupt_boot_upgrade_image.c -target=XCORE-200-EXPLORER -lquadflash -Wall
//
#include <xs1.h>
#include <platform.h>
#include <print.h>
#include <assert.h>
#include <quadflash.h>
#include <quadflashlib.h>

fl_QSPIPorts ports = {
  PORT_SQI_CS, PORT_SQI_SCLK, PORT_SQI_SIO, XS1_CLKBLK_1
};

fl_QuadDeviceSpec spec[] = { // IS25LQ016B
  { 0, 256, 8192, 3, 8, 0x9F, 0, 3, 0x9D4015, 0x20, 4096, 0x06, 0x04,
    PROT_TYPE_NONE, {{0,0},{0x00,0x00}}, 0x02, 0xEB, 1,
    SECTOR_LAYOUT_REGULAR, {4096,{0,{0}}}, 0x05, 0x01, 0x01
  }
};

void fl_int_write(unsigned char cmd,
                  unsigned int pageAddress, 
                  const unsigned char data[],
                  unsigned int num_bytes);

int main(void)
{
  fl_BootImageInfo info;
  int ret;

  ret = fl_connectToDevice(&ports, spec, 1);
  assert(ret == 0);

  ret = fl_getFactoryImage(&info);
  assert(ret == 0);

  ret = fl_getNextBootImage(&info);
  assert(ret == 0);

  ret = fl_setWritability(1);
  assert(ret == 0);

  // corrupt last word
  unsigned deadbeef = 0xDEADBEEF;
  fl_int_write(spec[0].programPageCommand,
               info.startAddress + info.size - 4,
               (void*)&deadbeef, 4);

  fl_setWritability(0);
  fl_disconnect();

  return 0;
}
