// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <quadflash.h>

#define _Bool int
#include <stdbool.h>

#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

unsigned fl_getDataPartitionBase()
{
  assert(0);
  return 1;
}

int fl_getNumSectors(void)
{
  assert(0);
  return 1;
}

void fl_int_read(unsigned char cmd,
                 unsigned int address,
                 unsigned char destination[num_bytes],
                 unsigned int num_bytes)
{
  assert(0);
}

int fl_getSectorAddress(int sectorNum)
{
  assert(0);
  return 1;
}

bool flash_is_sector_erased(unsigned address)
{
  assert(0);
  return false;
}

int flash_set_write_disable(void)
{
  assert(0);
  return -1;
}

int flash_write_page_async(unsigned address, const char page[])
{
  assert(0);
  return -1;
}

int flash_verify_page(unsigned address, const char page[])
{
  assert(0);
  return -1;
}

int flash_disconnect(void)
{
  assert(0);
  return -1;
}
