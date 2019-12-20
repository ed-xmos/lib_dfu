// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <quadflash.h>

#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

int fl_getFactoryImage(fl_BootImageInfo &bootImageInfo)
{
  assert(0);
  return 1;
}

int fl_getNextBootImage(fl_BootImageInfo &bootImageInfo)
{
  assert(0);
  return 1;
}

int fl_setWritability(int enable)
{
  assert(0);
  return 1;
}

int fl_getBusyStatus(void)
{
  assert(0);
  return 1;
}

void fl_int_eraseSector(unsigned char cmd, unsigned int sectorAddress)
{
  assert(0);
}

int fl_getSectorSize(int sectorNum)
{
  assert(0);
  return 1;
}

int fl_getNumSectors(void)
{
  assert(0);
  return 1;
}

int fl_getSectorAddress(int sectorNum)
{
  assert(0);
  return 1;
}

unsigned fl_getPageSize(void)
{
  assert(0);
  return 1;
}

int fl_readPage(unsigned int address, unsigned char data[])
{
  assert(0);
  return 1;
}

void fl_int_write(unsigned char cmd,
                  unsigned int pageAddress, 
                  const unsigned char data[num_bytes],
                  unsigned int num_bytes)
{
  assert(0);
}

int fl_connectToDevice(fl_QSPIPorts &ports, const fl_QuadDeviceSpec specs[], unsigned n)
{
  assert(0);
  return 1;
}

int fl_disconnect(void)
{
  assert(0);
  return 1;
}

unsigned fl_getDataPartitionBase()
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
