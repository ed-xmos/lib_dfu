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
#define DEBUG_PRINT_ENABLE_TEST 1
#include "debug_print.h"

#include "dfu.h"

fl_QSPIPorts ports = {
  PORT_SQI_CS, PORT_SQI_SCLK, PORT_SQI_SIO, XS1_CLKBLK_1
};

fl_QuadDeviceSpec spec[] = { // IS25LQ016B
  { 0, 256, 8192, 3, 8, 0x9F, 0, 3, 0x9D4015, 0x20, 4096, 0x06, 0x04,
    PROT_TYPE_NONE, {{0,0},{0x00,0x00}}, 0x02, 0xEB, 1,
    SECTOR_LAYOUT_REGULAR, {4096,{0,{0}}}, 0x05, 0x01, 0x01
  }
};

#define FACTORY_START 4096
#define FACTORY_SIZE 8192
#define MAX_IMAGE_SIZE 20480
#define FLASH_SIZE (MAX_IMAGE_SIZE * 2 + FACTORY_START + FACTORY_SIZE)

struct {
  struct {
    unsigned start;
    unsigned size;
    char contents[MAX_IMAGE_SIZE];
  } slot[2];
  int busy_countdown;
  bool write_enabled;
  bool page_erased[FLASH_SIZE / 256];
} flash = {{{0, 0, {0}}, {0, 0, {0}}}, 0, false, {false}};

int fl_connectToDevice(fl_QSPIPorts &ports,
                       const fl_QuadDeviceSpec specs[], unsigned n)
{
  return 0; // 0 indicates a matching flash device found and connected to
}

int fl_disconnect(void)
{
  return 0;
}

int fl_getFactoryImage(fl_BootImageInfo &bootImageInfo)
{
  bootImageInfo.startAddress = FACTORY_START;
  bootImageInfo.size = FACTORY_SIZE;
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

  assert(flash.write_enabled);
  assert(flash.busy_countdown == 0);
  if (sectorAddress >= flash.slot[0].start &&
      sectorAddress < flash.slot[0].start + flash.slot[0].size) {
    for (int i = 0; i < 16; i++) {
      int page_index = sectorAddress / 256 + i;
      int relative_page_address = (page_index * 256) - flash.slot[0].start;
      debug_printf("erase page %d (0x%X)\n", page_index, sectorAddress);
      assert(!flash.page_erased[page_index]);
      flash.page_erased[page_index] = true;
      memset(&flash.slot[0].contents[relative_page_address], 0xFF, 256);
    }
    flash.busy_countdown = 5;
  }
}

int fl_setWritability(int enable)
{
  flash.write_enabled = enable;
  return 0;
}

int fl_getBusyStatus(void)
{
  if (flash.busy_countdown > 0) {
    flash.busy_countdown--;
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

  assert(flash.write_enabled);
  assert(flash.busy_countdown == 0);
  assert(num_bytes == 256);
  assert(pageAddress >= flash.slot[0].start && // pad to whole pages
         pageAddress + num_bytes <= flash.slot[0].start + flash.slot[0].size + 256);

  assert(flash.page_erased[pageAddress / 256]);

  int relative_page_address = pageAddress - flash.slot[0].start;
  debug_printf("write relative page address %d (0x%02X)\n",
               relative_page_address, data[0]);

  memcpy(&flash.slot[0].contents[relative_page_address], data, num_bytes);

  flash.busy_countdown = 1;
}

int fl_readPage(unsigned int address, unsigned char data[])
{
  assert(address >= flash.slot[0].start && // allow extra sector for erased check
         address + 256 <= flash.slot[0].start + flash.slot[0].size + 4096);

  memcpy(data, &flash.slot[0].contents[address - flash.slot[0].start], 256);
  return 0;
}

unsigned fl_getDataPartitionBase()
{
  // TODO
  assert(0);
  return 1;
}

void fl_int_read(unsigned char cmd, 
                 unsigned int address, 
                 unsigned char destination[num_bytes], 
                 unsigned int num_bytes)
{
  // TODO
  assert(0);
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

void dnload(int partitions, const char images[2][MAX_IMAGE_SIZE],
            int block_size, int block_count, int tail_size)
{
  enum dfu_state state;

  state = dfu_getstate();
  assert(state == APP_IDLE);

  dfu_detach();
  state = dfu_getstate();
  assert(state == APP_DETACH);

  dfu_bus_reset(ports, spec);
  state = dfu_getstate();
  assert(state == DFU_IDLE);

  for (int p = 0; p < 2; p++) {
    if (partitions & (1 << p)) {
      const unsigned marker = (p << 15);
      for (int i = 0; i < block_count; i++) {
        debug_printf("dnload block %d 0x%04X\n",
                     i, marker | i);

        single_dnload_block(marker | i, block_size,
                            (const char*)&images[0][i * block_size]);
      }
      if (tail_size > 0) {
        debug_printf("dnload block %d 0x%04X (tail)\n",
                     block_count, marker | block_count);

        single_dnload_block(marker | block_count, tail_size,
                            (const char*)&images[0][block_count * block_size]);
      }
    }
  }

  dnload_zero();
}

void verify(int partitions, const char images[2][MAX_IMAGE_SIZE])
{
  for (int p = 0; p < 2; p++) {
    if (partitions & (1 << p)) {
      debug_printf("verify %d\n", p);
      for (int i = 0; i < flash.slot[p].size; i++) {
        if (images[p][i] != flash.slot[p].contents[i]) {
          debug_printf("byte %d mismatch: 0x%02X 0x%02X\n",
                        i, images[p][i], flash.slot[p].contents[i]);
          assert(0);
        }
      }
    }
  }
}

char images[2][MAX_IMAGE_SIZE];

int main(unsigned argc, char * unsafe argv[argc])
{
  int block_count = 0;
  int block_size = 0;
  int partitions = 0;
  int tail_size = 0;

  assert(argc == 5);

  unsafe {
    sscanf(argv[1], "%d", &block_size);
    sscanf(argv[2], "%d", &block_count);
    sscanf(argv[3], "%d", &tail_size);
    sscanf(argv[4], "%d", &partitions); // 1: boot only, 2: data only,
  }                                     // 3: boot and data

  flash.slot[0].start = FACTORY_START + FACTORY_SIZE;
  flash.slot[0].size = block_count * block_size + tail_size;
  flash.slot[1].start = ((flash.slot[0].start + flash.slot[0].size) / 4096 + 1) * 4096;
  flash.slot[1].size = flash.slot[0].size;

  debug_printf("+ %d * %d + %d = %d @ 0x%X, 0x%X [%d]\n",
               block_size, block_count, tail_size, flash.slot[0].size,
               flash.slot[0].start, flash.slot[1].start, partitions);

  random_sequence(images[0], flash.slot[0].size);
  random_sequence(images[1], flash.slot[1].size);

  dnload(partitions, images, block_size, block_count, tail_size);

  verify(partitions, images);

  printstr("PASS\n");
  return 0;
}
