// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
#ifndef DFU_FLASH_UNIT_TEST
#include <safestring.h>
#include <quadflash.h>
#include <quadflashlib.h>
#include "quadflash_extra.h"
#include "dfu_types.h"
#include "dfu_flash.h"

int flash_locate_boot_upgrade_slot(unsigned &address)
{
  fl_BootImageInfo info;

  if (fl_getFactoryImage(info) != 0)
    return 1;

  // rounding up to whole sectors as per fl_initImageWriteState
  if (fl_getNextBootImage(info) == 0)
    address = info.startAddress;
  else
    address = fl_roundAddressUpToWholeSector(info.startAddress + info.size);

  return 0;
}

int flash_locate_data_upgrade_slot(unsigned &address)
{
  fl_DataImageInfo info;

  if (fl_getFactoryDataImageNoChecksum(info) != 0)
    return 1;

  if (fl_getNextDataImageNoChecksum(info) == 0)
    address = info.startAddress;
  else
    address = fl_roundAddressUpToWholeSector(info.startAddress + info.size);

  return 0;
}

int flash_erase_sector_async(unsigned address)
{
  // protect first sector of boot partition
  if (address >= fl_getSectorAddress(0) && address < fl_getSectorEndAddress(0))
    return 1;

  // protect first sector of data partition
  if (address >= fl_getDataPartitionBase() &&
      address < fl_getSectorEndAddress(fl_getSectorContaining(fl_getDataPartitionBase())))
    return 2;

  // disallow wrap-around flash address in order to protect boot partition
  if (address >= fl_getFlashSize())
    return 3;

  if (fl_setWritability(1) != 0)
    return 4;

  unsafe {
    fl_int_eraseSector(g_flashAccess->sectorEraseCommand, address);
  }

  return 0;
}

bool flash_is_busy(void)
{
  return fl_getBusyStatus() != 0;
}

bool flash_is_first_whole_page_in_sector(unsigned address)
{
  int page_size = fl_getPageSize();

  if (address < page_size)
    return true;

  return fl_getSectorContaining(address - page_size) !=
         fl_getSectorContaining(address);
}

bool flash_is_sector_erased(unsigned address)
{
  unsigned page_address = address;
  int page_size = fl_getPageSize();
  while (fl_getSectorContaining(page_address) == fl_getSectorAtOrAfter(address)) {
    char page[DFU_PAGE_SIZE_MAX_BYTES];
    fl_readPage(page_address, page);
    for (int i = 0; i < page_size; i++) {
      if (page[i] != 0xFF)
        return false;
    }
    page_address += page_size;
  }
  return true;
}

int flash_set_write_disable(void)
{
  return fl_setWritability(0);
}

int flash_write_page_async(unsigned address, const char page[])
{
  int page_size = fl_getPageSize();

  // protect first sector of boot partition
  if (address >= fl_getSectorAddress(0) && address < fl_getSectorEndAddress(0))
    return 1;

  // protect first sector of data partition
  if (address >= fl_getDataPartitionBase() &&
      address < fl_getSectorEndAddress(fl_getSectorContaining(fl_getDataPartitionBase())))
    return 2;

  // disallow wrap-around flash address in order to protect boot partition
  if (address >= fl_getFlashSize())
    return 3;

  if (fl_setWritability(1) != 0)
    return 4;

  unsafe {
    fl_int_write(g_flashAccess->programPageCommand, address, page, page_size);
  }

  return 0;
}

int flash_verify_page(unsigned address, const char page[])
{
  int page_size = fl_getPageSize();
  char verify[DFU_PAGE_SIZE_MAX_BYTES];

  fl_readPage(address, verify);
  return safememcmp(verify, page, page_size);
}

int flash_get_data_partition_base(void)
{
  return fl_getDataPartitionBase();
}

int flash_get_page_size(void)
{
  return fl_getPageSize();
}

int flash_get_size(void)
{
  return fl_getFlashSize();
}

#endif
