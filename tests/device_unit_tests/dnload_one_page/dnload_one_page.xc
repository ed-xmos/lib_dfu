// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <platform.h>
#include <print.h>
#include <string.h>
#include <quadflash.h>

#define _Bool int
#include <stdbool.h>

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

char g_flash_upgrade_slot[8 * DFU_BLOCK_SIZE_MAX_BYTES];

const char g_image[8][DFU_BLOCK_SIZE_MAX_BYTES] = {
  {0xE7, 0x8E, 0xF1, 0xF7, 0x72, 0x01, 0xC5, 0x27, 0x26, 0x19, 0x5F, 0x2B, 0x1A, 0xBF, 0x92, 0xC7,
   0x7F, 0x33, 0x70, 0x21, 0xB7, 0x82, 0xEF, 0x2E, 0x4C, 0x2D, 0x5E, 0x96, 0xBC, 0xD1, 0x36, 0x5D},
  {0xCD, 0xF4, 0x10, 0x12, 0x39, 0xF2, 0x69, 0x0C, 0xD7, 0x63, 0x7A, 0x9C, 0x4A, 0x6C, 0xD0, 0x49,
   0x65, 0xDE, 0xA7, 0x8B, 0x7C, 0x5F, 0xEF, 0x6E, 0xB5, 0xA3, 0x32, 0x2C, 0x13, 0xF3, 0xE6, 0xB5},
  {0x80, 0xF9, 0xE2, 0x40, 0x05, 0xD2, 0x96, 0xA8, 0x79, 0xEF, 0x07, 0x2D, 0xF1, 0xE9, 0xCB, 0x63,
   0x01, 0x4D, 0xAC, 0x4A, 0x44, 0x0E, 0x7F, 0xDC, 0x91, 0x9E, 0xC2, 0xFB, 0x38, 0x7D, 0x0B, 0x30},
  {0x79, 0x44, 0x82, 0x64, 0xF5, 0x58, 0x1B, 0x3F, 0xEE, 0x84, 0x0D, 0xD5, 0x75, 0x78, 0x14, 0x04,
   0x92, 0x8B, 0x98, 0x0F, 0xD0, 0x24, 0x2B, 0x93, 0x4B, 0x1F, 0xD1, 0x1A, 0x13, 0xB0, 0x9C, 0xD6},
  {0x0D, 0xB4, 0x4B, 0x65, 0xF2, 0x4D, 0xD9, 0xBA, 0x4B, 0xCD, 0x72, 0x68, 0x83, 0x6D, 0x1C, 0x1E,
   0x1C, 0x55, 0xB5, 0x78, 0x7B, 0xC5, 0x97, 0xB2, 0xA5, 0x08, 0x22, 0xE1, 0x31, 0x36, 0xEE, 0xAD},
  {0xAE, 0x23, 0x9D, 0xEF, 0x4F, 0xA7, 0x27, 0x44, 0x6F, 0x2E, 0xB1, 0xD8, 0x7C, 0x95, 0xC8, 0x2A,
   0x99, 0xE4, 0x81, 0x53, 0x3A, 0x88, 0x3D, 0x77, 0xA2, 0xCA, 0x0D, 0xDB, 0xFA, 0x58, 0x90, 0xE6},
  {0x70, 0x10, 0xB0, 0xCE, 0x14, 0xD4, 0x24, 0x7F, 0x7E, 0xCD, 0x26, 0xF5, 0xE1, 0x01, 0xCF, 0xAC,
   0x61, 0x90, 0x49, 0x96, 0x13, 0x2C, 0x95, 0x9C, 0xB0, 0x56, 0xFF, 0x5D, 0x96, 0xEB, 0x8D, 0xFB},
  {0x44, 0x77, 0x02, 0xBA, 0xEC, 0x31, 0xD5, 0x4F, 0x4F, 0x8D, 0x89, 0xEA, 0x3A, 0xD2, 0xD2, 0x1E,
   0x49, 0x40, 0x7A, 0x15, 0x0D, 0xB4, 0xCA, 0xFD, 0xCC, 0xFE, 0x0F, 0x98, 0xEB, 0x11, 0x7C, 0xAB},
};

bool g_flash_erased = false;
bool g_flash_write_enabled = false;
int g_flash_working = 0;

const unsigned g_factory_start = 4096;
const unsigned g_factory_size = 8192;
const unsigned g_upgrade_start = 4096 + 8192;

int fl_connectToDevice(fl_QSPIPorts &ports, const fl_QuadDeviceSpec specs[], unsigned n)
{
  return 0; // 0 indicates a matching flash device found and connected to
}

int fl_disconnect(void)
{
  return 0;
}

int fl_getFactoryImage(fl_BootImageInfo &bootImageInfo)
{
  assert(sizeof(g_image) == 256); // sanity check
  bootImageInfo.startAddress = g_factory_start;
  bootImageInfo.size = g_factory_size;
  bootImageInfo.factory = 1;
  return 0; // 0 represents a valid factory image
}

int fl_getNextBootImage(fl_BootImageInfo &bootImageInfo)
{
  return 1; // 1 simulates an empty upgrade slot
}

void fl_int_eraseSector(unsigned char cmd, unsigned int sectorAddress)
{
  printstr("fl_int_eraseSector\n");

  assert(g_flash_write_enabled);
  assert(g_flash_working == 0);
  if (sectorAddress == g_upgrade_start) {
    assert(!g_flash_erased);
    g_flash_erased = true;
    g_flash_working = 5;
  }
}

int fl_setWritability(int enable)
{
  g_flash_write_enabled = enable;
  return 0;
}

int fl_getBusyStatus(void)
{
  if (g_flash_working > 0) {
    g_flash_working--;
    return 1;
  }
  else {
    return 0;
  }
}

unsigned fl_getPageSize(void)
{
  return 256;
}

int fl_getSectorSize(int sectorNum)
{
  return 4096;
}

int fl_getNumSectors(void)
{
  return 512;
}

int fl_getSectorAddress(int sectorNum)
{
  return sectorNum * 4096;
}

void fl_int_write(unsigned char cmd,
                  unsigned int pageAddress, 
                  const unsigned char data[num_bytes],
                  unsigned int num_bytes)
{
  printstr("fl_int_write\n");

  assert(g_flash_write_enabled);
  assert(g_flash_working == 0);
  assert(g_flash_erased);
  assert(num_bytes == 256);
  assert(pageAddress >= g_upgrade_start &&
         pageAddress + num_bytes <= g_upgrade_start + sizeof(g_image));

  memcpy(&g_flash_upgrade_slot[pageAddress - g_upgrade_start], data, num_bytes);

  g_flash_working = 1;
}

int fl_readPage(unsigned int address, unsigned char data[])
{
  assert(address >= g_upgrade_start &&
         address + 256 <= g_upgrade_start + sizeof(g_image));

  memcpy(data, &g_flash_upgrade_slot[address - g_upgrade_start], 256);
  return 0;
}

int main(void)
{
  enum dfu_state state;
  enum dfu_status status;
  unsigned timeout;

  state = dfu_getstate();
  assert(state == APP_IDLE);

  dfu_detach();
  state = dfu_getstate();
  assert(state == APP_DETACH);

  dfu_bus_reset(g_ports, g_spec);
  state = dfu_getstate();
  assert(state == DFU_IDLE);

  for (int i = 0; i < sizeof(g_image) / DFU_BLOCK_SIZE_MAX_BYTES; i++) {
    printintln(i);

    dfu_dnload(i, DFU_BLOCK_SIZE_MAX_BYTES, g_image[i]);

    do {
      {status, state, timeout} = dfu_getstatus();
      assert(status == DFU_OK);
      delay_microseconds(1);
    } while (state == DFU_DNBUSY);

    assert(state == DFU_DNLOAD_IDLE);
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
