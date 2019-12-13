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

#define IMAGE_BLOCK_COUNT 128

char g_image[IMAGE_BLOCK_COUNT][DFU_BLOCK_SIZE_MAX_BYTES];

char g_flash_upgrade_slot[IMAGE_BLOCK_COUNT * DFU_BLOCK_SIZE_MAX_BYTES];

bool g_flash_erased = false;
bool g_flash_write_enabled = false;
int g_flash_working = 0;

const unsigned g_factory_start = 4096;
const unsigned g_factory_size = 8192;
const unsigned g_upgrade_start = 4096 + 8192;
const unsigned g_upgrade_size = IMAGE_BLOCK_COUNT * DFU_BLOCK_SIZE_MAX_BYTES;

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

void random_sequence(char seq[], int length)
{
  for (int i = 0; i < length; i++) {
    unsigned x;
    crc32(x, -1, 0xEB31D82E);
    seq[i] = x;
  }
}

int main(void)
{
  enum dfu_state state;
  enum dfu_status status;
  unsigned timeout;

  for (int i = 0; i < sizeof(g_image) / DFU_BLOCK_SIZE_MAX_BYTES; i++) {
    random_sequence(g_image[i], DFU_BLOCK_SIZE_MAX_BYTES);
  }

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
