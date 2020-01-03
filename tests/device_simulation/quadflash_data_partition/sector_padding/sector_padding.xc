// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <print.h>

#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

#include "quadflash_data_partition.h"

fl_QuadDeviceSpec spec[] = { // IS25LQ016B
  { 0, 256, 8192, 3, 8, 0x9F, 0, 3, 0x9D4015, 0x20, 4096, 0x06, 0x04,
    PROT_TYPE_NONE, {{0,0},{0x00,0x00}}, 0x02, 0xEB, 1,
    SECTOR_LAYOUT_REGULAR, {4096,{0,{0}}}, 0x05, 0x01, 0x01
  }
};

int fl_getNumSectors(void)
{
  return 512;
}

int fl_getSectorAddress(int sectorNum)
{
  return sectorNum * 4096;
}

unsigned last_read_address = 0;

void fl_int_read(unsigned char cmd, 
                 unsigned int address, 
                 unsigned char destination[num_bytes], 
                 unsigned int num_bytes)
{
  last_read_address = address;
}

void test(unsigned size, unsigned address)
{
  fl_DataImageInfo info;

  info.startAddress = 0;
  info.size = size;
  info.factory = 0;
  info.version = 1; // non-zero version is sensible

  last_read_address = 0;

  // function will fail on a CRC check
  // but we only need to see address it tried to read from
  fl_getNextDataImage(info);

  assert(last_read_address == address);
}

int main(void)
{
  fl_saveSpecPointer(spec);

  test(1, 4096);
  test(4095, 4096);
  test(4096, 4096);
  test(4097, 8192);
  test(8191, 8192);
  test(8192, 8192);

  printstr("PASS\n");
  return 0;
}
