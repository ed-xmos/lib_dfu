// Copyright (c) 2020, XMOS Ltd, All rights reserved
#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

unsigned fl_getDataPartitionBase()
{
  assert(0);
  return ~0;
}

int fl_getNumSectors(void)
{
  assert(0);
  return -1;
}

int fl_getSectorAddress(int sectorNum)
{
  assert(0);
  return -1;
}

void fl_int_read(unsigned char cmd,
                 unsigned int address,
                 unsigned char destination[num_bytes],
                 unsigned int num_bytes)
{
  assert(0);
}
