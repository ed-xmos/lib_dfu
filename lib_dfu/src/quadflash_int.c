// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stdlib.h>
#include <quadflash.h>
#include "quadflash_int.h"

static const fl_QuadDeviceSpec* g_flashAccess = NULL;

void fl_saveSpecPointer(const fl_QuadDeviceSpec spec[1])
{
  g_flashAccess = spec;
}

int fl_getSectorEndAddress(int sectorNum)
{
  return fl_getSectorAddress(sectorNum) + fl_getSectorSize(sectorNum);
}

int fl_getSectorContaining(unsigned address)
{
  unsigned numSectors = fl_getNumSectors();
  unsigned sector;
  for (sector = 0; sector < numSectors; sector++)
  {
    if (fl_getSectorEndAddress(sector) > address)
    {
      return sector;
    }
  }
  return -1;
}

int fl_getSectorAtOrAfter(unsigned address)
{
  unsigned numSectors = fl_getNumSectors();
  unsigned sector;
  for (sector = 0; sector < numSectors; sector++)
  {
    if (fl_getSectorAddress(sector) >= address)
    {
      return sector;
    }
  }
  return -1;
}
