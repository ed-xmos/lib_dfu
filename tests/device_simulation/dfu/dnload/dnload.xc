// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
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

#include "quadflash_data_partition.h"
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

#define MAX_IMAGE_SIZE 20480

struct {
  struct {
    unsigned base;
    unsigned f_start;
    unsigned f_size;
    unsigned u_start;
    unsigned u_size;
    char u_contents[MAX_IMAGE_SIZE];
  } partitions[2];
  int busy_countdown;
  bool write_enabled;
  bool page_erased[8192];
} fl;

const char labels[2][5] = {"boot", "data"};

void layout_flash(int block_count, int block_size, int tail_size)
{
  debug_printf("image blocks %d x %d bytes + %d bytes tail\n",
               block_count, block_size, tail_size);

  fl.partitions[0].base = 0;
  fl.partitions[1].base = 1048576;

  for (int p = 0; p < 2; p++) {
    fl.partitions[p].f_start = fl.partitions[p].base + 4096;
    fl.partitions[p].f_size = 256;
    fl.partitions[p].u_start = fl.partitions[p].f_start + 4096;
    fl.partitions[p].u_size = block_count * block_size + tail_size;
    memset(fl.partitions[p].u_contents, 0, MAX_IMAGE_SIZE);

    debug_printf("%s partition: factory 0x%X (%d), upgrade 0x%X (%d)\n",
                 labels[p], fl.partitions[p].f_start, fl.partitions[p].f_size,
                 fl.partitions[p].u_start, fl.partitions[p].u_size);
  }

  fl.busy_countdown = 0;
  fl.write_enabled = false;

  memset(fl.page_erased, 0, sizeof(fl.page_erased));
}

int fl_getFactoryImage(fl_BootImageInfo &bootImageInfo)
{
  bootImageInfo.startAddress = fl.partitions[0].f_start;
  bootImageInfo.size = fl.partitions[0].f_size;
  bootImageInfo.factory = 1;
  return 0; // 0 represents a valid factory image
}

int fl_getFactoryDataImage(fl_DataImageInfo &dataImageInfo)
{
  dataImageInfo.startAddress = fl.partitions[1].f_start;
  dataImageInfo.size = fl.partitions[1].f_size;
  dataImageInfo.factory = 1;
  return 0;
}

int fl_getNextBootImage(fl_BootImageInfo &bootImageInfo)
{
  return 1; // 1 simulates an empty upgrade slot
}

int fl_getNextDataImage(fl_DataImageInfo &dataImageInfo)
{
  return 1;
}

void fl_int_eraseSector(unsigned char cmd, unsigned int sectorAddress)
{
  debug_printf("fl_int_eraseSector 0x%X\n", sectorAddress);

  assert(cmd == 0x20);
  assert(fl.write_enabled);
  assert(fl.busy_countdown == 0);

  for (int i = 0; i < 16; i++) {
    int page_address = sectorAddress + 256 * i;
    int page_index = sectorAddress / 256 + i;

    for (int p = 0; p < 2; p++) {
      if (page_address >= fl.partitions[p].u_start &&
          page_address < fl.partitions[p].u_start + fl.partitions[p].u_size) {

        int contents_offset = sectorAddress - fl.partitions[p].u_start + 256 * i;
        debug_printf("erase %s upgrade offset 0x%X (flash page %d)\n",
                     labels[p], contents_offset, page_index);

        memset(&fl.partitions[p].u_contents[contents_offset], 0xFF, 256);
      }
    }
    fl.page_erased[page_index] = true;
  }
  fl.busy_countdown = 5;
}

void fl_int_write(unsigned char cmd,
                  unsigned int pageAddress,
                  const unsigned char data[num_bytes],
                  unsigned int num_bytes)
{
  debug_printf("fl_int_write\n");

  assert(cmd == 0x02);
  assert(fl.write_enabled);
  assert(fl.busy_countdown == 0);
  assert(num_bytes == 256); // only expect whole page writes
  assert(fl.page_erased[pageAddress / 256]);

  for (int p = 0; p < 2; p++) {
    if (pageAddress >= fl.partitions[p].u_start &&
        pageAddress < fl.partitions[p].u_start + fl.partitions[p].u_size) {

      int contents_offset = pageAddress - fl.partitions[p].u_start;
      debug_printf("write %s upgrade offset 0x%X (flash page %d)\n",
                   labels[p], contents_offset, pageAddress / 256);

      memcpy(&fl.partitions[p].u_contents[contents_offset], data, num_bytes);
    }
  }

  fl.busy_countdown = 1;
}

int fl_readPage(unsigned int address, unsigned char data[])
{
  debug_printf("fl_readPage 0x%X\n", address);

  assert(fl.busy_countdown == 0);

  for (int p = 0; p < 2; p++) {
    if (address >= fl.partitions[p].u_start &&
        address < fl.partitions[p].u_start + fl.partitions[p].u_size) {

      int contents_offset = address - fl.partitions[p].u_start;
      debug_printf("read %s upgrade offset 0x%X (flash page %d)\n",
                   labels[p], contents_offset, address / 256);

      memcpy(data, &fl.partitions[p].u_contents[contents_offset], 256);
    }
  }

  return 0;
}

void fl_int_read(unsigned char cmd,
                 unsigned int address,
                 unsigned char destination[num_bytes],
                 unsigned int num_bytes)
{
  debug_printf("fl_int_read 0x%X %d\n", address, num_bytes);
  assert(0);
}

unsigned fl_getDataPartitionBase()
{
  return fl.partitions[1].base;
}

int fl_setWritability(int enable)
{
  fl.write_enabled = enable;
  return 0;
}

int fl_getBusyStatus(void)
{
  if (fl.busy_countdown > 0) {
    debug_printf("busy countdown %d\n", fl.busy_countdown);
    fl.busy_countdown--;
    return 1;
  }
  else {
    return 0;
  }
}

int fl_connectToDevice(fl_QSPIPorts &ports,
                       const fl_QuadDeviceSpec specs[], unsigned n)
{
  return 0; // 0 indicates a matching f device found and connected to
}

int fl_disconnect(void)
{
  return 0;
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

void fl_saveSpecPointer(const fl_QuadDeviceSpec spec[1])
{
  // nothing
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
  struct dfu_getstatus ret;
  enum dfu_state state;

  dfu_dnload(block_num, block_size, block);
  state = dfu_getstate();
  assert(state == DFU_DNLOAD_SYNC);

  do {
    ret = dfu_getstatus();
    assert(ret.status == DFU_OK);
    delay_microseconds(1);
  } while (ret.state == DFU_DNBUSY);

  assert(ret.state == DFU_DNLOAD_IDLE);
}

void dnload_zero(void)
{
  struct dfu_getstatus ret;
  enum dfu_state state;
  char block[DFU_BLOCK_SIZE_MAX_BYTES];

  dfu_dnload(0, 0, block);
  state = dfu_getstate();
  assert(state == DFU_MANIFEST_SYNC);

  do {
    ret = dfu_getstatus();
    assert(ret.status == DFU_OK);
    delay_microseconds(1);
  } while (ret.state == DFU_MANIFEST);

  assert(ret.state == DFU_IDLE);
  assert(ret.status == DFU_OK);
}

void dnload(int partitions, const char images[2][MAX_IMAGE_SIZE],
            int block_size, int block_count, int tail_size, int repeats)
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

  for (int i = 0; i < repeats; i++) {
    for (int p = 0; p < 2; p++) {
      if (partitions & (1 << p)) {
        const unsigned marker = (p << 15);
        for (int i = 0; i < block_count; i++) {
          debug_printf("dnload block %d 0x%04X (%d bytes)\n",
                       i, marker | i, block_size);

          single_dnload_block(marker | i, block_size,
                              (const char*)&images[p][i * block_size]);
        }
        if (tail_size > 0) {
          debug_printf("dnload block %d 0x%04X (tail %d bytes)\n",
                       block_count, marker | block_count, tail_size);

          single_dnload_block(marker | block_count, tail_size,
                              (const char*)&images[p][block_count * block_size]);
        }
        debug_printf("dnload zero\n");
        dnload_zero();
      }
    }
  }
}

void verify(int partitions, const char images[2][MAX_IMAGE_SIZE])
{
  for (int p = 0; p < 2; p++) {
    if (partitions & (1 << p)) {
      debug_printf("verify %d\n", p);
      for (int i = 0; i < fl.partitions[p].u_size; i++) {
        if (images[p][i] != fl.partitions[p].u_contents[i]) {
          debug_printf("byte %d mismatch: 0x%02X 0x%02X\n",
                        i, images[p][i], fl.partitions[p].u_contents[i]);
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
  int repeats = 0;

  assert(argc == 6);

  unsafe {
    sscanf(argv[1], "%d", &block_size);
    sscanf(argv[2], "%d", &block_count);
    sscanf(argv[3], "%d", &tail_size);
    sscanf(argv[4], "%d", &partitions);
    sscanf(argv[5], "%d", &repeats);
  }

  layout_flash(block_count, block_size, tail_size);

  random_sequence(images[0], fl.partitions[0].u_size);
  random_sequence(images[1], fl.partitions[1].u_size);

  dnload(partitions, images, block_size, block_count, tail_size, repeats);

  verify(partitions, images);

  printstr("PASS\n");
  return 0;
}
