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

int g_connected = 0;

int fl_connectToDevice(fl_QSPIPorts &ports, const fl_QuadDeviceSpec specs[], unsigned n)
{
  unsafe {
    assert(n == 1);
    assert(specs == p_spec);
    assert((int)ports.qspiCS == (int)p_ports->qspiCS);
    assert((int)ports.qspiSCLK == (int)p_ports->qspiSCLK);
    assert((int)ports.qspiSIO == (int)p_ports->qspiSIO);
    assert((int)ports.qspiClkblk == (int)p_ports->qspiClkblk);
  }
  g_connected = 1;
  return 0;
}

int fl_disconnect(void)
{
  assert(g_connected);
  g_connected = 0;
  return 0;
}

int fl_getFactoryImage(fl_BootImageInfo &bootImageInfo)
{
  assert(0); // call not expected
  return 1;
}

int fl_getNextBootImage(fl_BootImageInfo &bootImageInfo)
{
  assert(0); // call not expected
  return 1;
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

  printstr("PASS\n");
  return 0;
}
