// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stdlib.h>
#include <quadflash.h>
#include "quadflash_internal.h"

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
