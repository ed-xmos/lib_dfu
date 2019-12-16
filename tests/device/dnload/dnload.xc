// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <platform.h>
#include <stdio.h>
#include <print.h>
#include <string.h>
#include <quadflash.h>

#define _Bool int
#include <stdbool.h>

#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

#define DEBUG_UNIT TEST
#define DEBUG_PRINT_ENABLE_TEST 0
#include "debug_print.h"

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

#define MAX_IMAGE_SIZE 40000

char g_image[MAX_IMAGE_SIZE];

char g_flash_upgrade_slot[MAX_IMAGE_SIZE];

bool g_page_erased[MAX_IMAGE_SIZE / 256] = {false};
bool g_flash_write_enabled = false;
int g_flash_working = 0;

const unsigned g_factory_start = 4096;
const unsigned g_factory_size = 8192;
const unsigned g_upgrade_start = 4096 + 8192;
unsigned g_upgrade_size = 0;

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
  debug_printf("fl_int_eraseSector\n");

  assert(g_flash_write_enabled);
  assert(g_flash_working == 0);
  if (sectorAddress >= g_upgrade_start &&
      sectorAddress < g_upgrade_start + g_upgrade_size) {
    for (int i = 0; i < 16; i++) {
      int upgrade_image_page_index = (sectorAddress - g_upgrade_start) / 256 + i;
      assert(!g_page_erased[upgrade_image_page_index]);
      g_page_erased[upgrade_image_page_index] = true;
      memset(&g_flash_upgrade_slot[sectorAddress - g_upgrade_start + i * 256], 0xFF, 256);
    }
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
  debug_printf("fl_int_write\n");

  assert(g_flash_write_enabled);
  assert(g_flash_working == 0);
  assert(num_bytes == 256);
  assert(pageAddress >= g_upgrade_start && // pad to whole pages
         pageAddress + num_bytes <= g_upgrade_start + g_upgrade_size + 256);

  assert(g_page_erased[(pageAddress - g_upgrade_start) / 256]);

  memcpy(&g_flash_upgrade_slot[pageAddress - g_upgrade_start], data, num_bytes);

  g_flash_working = 1;
}

int fl_readPage(unsigned int address, unsigned char data[])
{
  assert(address >= g_upgrade_start && // allow an extra sector for erased check
         address + 256 <= g_upgrade_start + g_upgrade_size + 4096);

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

void single_dnload_block(int block_num, size_t block_size, const char block[])
{
  enum dfu_state state;
  enum dfu_status status;
  unsigned timeout;

  dfu_dnload(block_num, block_size, block);
  state = dfu_getstate();
  assert(state == DFU_DNLOAD_SYNC);

  do {
    {status, state, timeout} = dfu_getstatus();
    assert(status == DFU_OK);
    delay_microseconds(1);
  } while (state == DFU_DNBUSY);

  assert(state == DFU_DNLOAD_IDLE);
}

void dnload_zero(void)
{
  enum dfu_state state;
  enum dfu_status status;
  unsigned timeout;
  char block[DFU_BLOCK_SIZE_MAX_BYTES];

  dfu_dnload(0, 0, block);
  state = dfu_getstate();
  assert(state == DFU_MANIFEST_SYNC);

  do {
    {status, state, timeout} = dfu_getstatus();
    assert(status == DFU_OK);
    delay_microseconds(1);
  } while (state == DFU_MANIFEST);

  assert(state == DFU_IDLE);
  assert(status == DFU_OK);
}

void dnload(int block_size, int block_count, int tail_size)
{
  enum dfu_state state;

  state = dfu_getstate();
  assert(state == APP_IDLE);

  dfu_detach();
  state = dfu_getstate();
  assert(state == APP_DETACH);

  dfu_bus_reset(g_ports, g_spec);
  state = dfu_getstate();
  assert(state == DFU_IDLE);

  for (int i = 0; i < block_count; i++) {
    debug_printf("%d\n", i);
    single_dnload_block(i, block_size,
                        (const char*)&g_image[i * block_size]);
  }

  if (tail_size > 0) {
    debug_printf("%d\n", block_count);
    single_dnload_block(block_count, tail_size,
                        (const char*)&g_image[block_count * block_size]);
  }

  dnload_zero();
}

void verify(void)
{
  for (int i = 0; i < g_upgrade_size; i++) {
    if (g_image[i] != g_flash_upgrade_slot[i]) {
      debug_printf("byte %d mismatch: 0x%02X 0x%02X\n",
             i, g_image[i], g_flash_upgrade_slot[i]);
      assert(0);
    }
  }
}

int main(unsigned argc, char * unsafe argv[argc])
{
  int block_count = 0;
  int block_size = 0;
  int tail_size = 0;

  assert(argc == 4);

  unsafe {
    sscanf(argv[1], "%d", &block_size);
    sscanf(argv[2], "%d", &block_count);
    sscanf(argv[3], "%d", &tail_size);
  }
  g_upgrade_size = block_count * block_size + tail_size;
  debug_printf("+ %d * %d + %d (%d)\n", block_size, block_count, tail_size,
                                  g_upgrade_size);

  random_sequence(g_image, g_upgrade_size);

  dnload(block_size, block_count, tail_size);

  verify();

  printstr("PASS\n");
  return 0;
}
