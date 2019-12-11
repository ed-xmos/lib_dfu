// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stddef.h>
#include <quadflash.h>

#define _Bool int
#include <stdbool.h>

#include <print.h>
#include "quadflash_int.h"
#include "dfu_flash.h"

static unsigned char sector_erase_command = 0;

int flash_connect(fl_QSPIPorts &ports, const fl_QuadDeviceSpec spec[1])
{
  int ret;

  ret = fl_connectToDevice(ports, spec, 1);
  if (ret != 0)
    return ret;

  fl_saveSpecPointer(spec);
  sector_erase_command = spec[0].sectorEraseCommand;

  return 0;
}

int flash_disconnect(void)
{
  return fl_disconnect();
}

int flash_locate_upgrade_slot(unsigned &address)
{
  fl_BootImageInfo factory, upgrade;

  int ret = fl_getFactoryImage(factory);
  if (ret != 0)
    return ret;

  if (fl_getNextBootImage(upgrade) == 0) {
    address = upgrade.startAddress;
  }
  else {
    address = factory.startAddress + factory.size;
    address = fl_getSectorAddress(fl_getSectorAtOrAfter(address)); // sector aligned
  }

  return 0;
}

int flash_is_upgrade_slot_valid(bool &valid)
{
  fl_BootImageInfo image;

  int ret = fl_getFactoryImage(image);
  if (ret != 0)
    return ret;

  valid = (fl_getNextBootImage(image) == 0);

  return 0;
}

bool flash_is_first_whole_page_in_sector(unsigned address)
{
  return true;
}

int flash_erase_sector_async(unsigned address)
{
  if (fl_setWritability(1) != 0)
    return 1;

  fl_int_eraseSector(sector_erase_command, address);

  return 0;
}

int flash_write_page_async(unsigned address, const char page[])
{
  // TODO
  return 0;
}

bool flash_is_busy(void)
{
  return fl_getBusyStatus() != 0;
}

int flash_set_write_disable(void)
{
  return fl_setWritability(0);
}
