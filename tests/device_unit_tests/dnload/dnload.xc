// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <platform.h>
#include <print.h>
#include <string.h>
#include <quadflash.h>

#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

#include "dfu.h"

fl_QSPIPorts g_ports = {
  PORT_SQI_CS, PORT_SQI_SCLK, PORT_SQI_SIO, XS1_CLKBLK_1
};

fl_QuadDeviceSpec g_spec[] = { // IS25LQ016B
  { 0, 256, 8192, 3, 8, 0x9F, 0, 3, 0x9D4015, 0x20, 4096, 0x06, 0x04,
    PROT_TYPE_NONE, {{0,0},{0x00,0x00}}, 0x02, 0xEB, 1,
    SECTOR_LAYOUT_REGULAR, {4096,{0,{0}}}, 0x05, 0x01, 0x01
  }
};

unsafe {
  fl_QSPIPorts * unsafe p_ports = (fl_QSPIPorts * unsafe)&g_ports;
  fl_QuadDeviceSpec * unsafe p_spec = (fl_QuadDeviceSpec * unsafe)&g_spec;
}

char g_image[4][DFU_BLOCK_SIZE_MAX_BYTES] = {
  {0x3A, 0x53, 0x27, 0xF5, 0x04, 0x99, 0x01, 0xC5, 0xDE, 0xDD, 0x35, 0x5F, 0x58, 0x63, 0xB9, 0x3F,
   0x52, 0xF4, 0x43, 0x7A, 0xDB, 0xD1, 0x4A, 0xFB, 0x04, 0x55, 0xD8, 0xAE, 0x6D, 0xA9, 0x05, 0x79},
  {0xC5, 0x7D, 0x02, 0x43, 0xA8, 0xBF, 0x0D, 0x28, 0x57, 0x2D, 0x33, 0x8B, 0xB3, 0x05, 0x53, 0x81,
   0xE1, 0xA1, 0x2E, 0x99, 0x61, 0xCC, 0x88, 0x60, 0x94, 0x11, 0x34, 0x18, 0x5A, 0xE5, 0xC1, 0xA9},
  {0x2D, 0x19, 0x13, 0xC5, 0x3B, 0x34, 0xA4, 0x2F, 0xBA, 0xF1, 0xF4, 0x8E, 0x50, 0x5E, 0x93, 0x4C,
   0x88, 0x53, 0x07, 0xEC, 0x4E, 0x50, 0x34, 0xBC, 0x29, 0xDA, 0x8F, 0x79, 0xD5, 0x88, 0xBF, 0xDC},
  {0x2F, 0x50, 0xA1, 0xED, 0xC4, 0x22, 0xF0, 0x74, 0xF4, 0x7F, 0xF3, 0xCF, 0xD3, 0xE6, 0x56, 0xE6,
   0x82, 0x68, 0xE6, 0x09, 0x81, 0xA4, 0x13, 0x9D, 0x86, 0xCF, 0x44, 0x96, 0xE8, 0x67, 0xE9, 0x10},
};

int fl_connectToDevice(fl_QSPIPorts &ports, const fl_QuadDeviceSpec specs[], unsigned n)
{
  return 0; // 0 indicates a matching flash device found and connected to
}

int fl_getFactoryImage(fl_BootImageInfo &bootImageInfo)
{
  return 0; // 0 represents a valid factory image
}

int fl_getNextBootImage(fl_BootImageInfo &bootImageInfo)
{
  return 1; // 1 simulates an empty upgrade slot
}

int main(void)
{
  enum dfu_state state;
  enum dfu_status status;
  unsigned timeout;

  state = dfu_getstate();
  assert(state == APP_IDLE);

  dfu_detach();
  {status, state, timeout} = dfu_getstatus();
  assert(state == APP_DETACH);
  assert(status == DFU_OK);

  dfu_bus_reset(g_ports, g_spec);
  {status, state, timeout} = dfu_getstatus();
  assert(state == DFU_IDLE);
  assert(status == DFU_OK);

  for (int i = 0; i < 4; i++) {
    printintln(i);

    dfu_dnload(i, DFU_BLOCK_SIZE_MAX_BYTES, g_image[i]);
    state = dfu_getstate();
    assert(state == DFU_DNLOAD_SYNC);

    {status, state, timeout} = dfu_getstatus();
    assert(status == DFU_OK);
    assert(state == DFU_DNBUSY);

    delay_microseconds(1);

    {status, state, timeout} = dfu_getstatus();
    assert(status == DFU_OK);
    assert(state == DFU_DNLOAD_IDLE);
    delay_microseconds(1);
  }

  dfu_dnload(0, 0, g_image[0]);
  state = dfu_getstate();
  assert(state == DFU_MANIFEST_SYNC);

  {status, state, timeout} = dfu_getstatus();
  assert(state == DFU_IDLE);
  assert(status == DFU_OK);

  printstr("PASS\n");
  return 0;
}
