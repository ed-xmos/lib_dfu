// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stddef.h>
#include <assert.h>
#include <safestring.h>
#include <print.h>

#define _Bool int
#include <stdbool.h>

#define DEBUG_UNIT DFU_FLASH
#define DEBUG_PRINT_ENABLE_DFU_FLASH 0
#include "debug_print.h"

#include <quadflash.h>
#include "quadflash_internal.h"
#include "quadflash_data_partition.h"
#include "dfu_flash.h"

static unsigned char sector_erase_command = 0;
static unsigned char program_page_command = 0;

int flash_connect(fl_QSPIPorts &ports, const fl_QuadDeviceSpec spec[1])
{
  int ret;

  ret = fl_connectToDevice(ports, spec, 1);
  if (ret != 0)
    return ret;

  fl_saveSpecPointer(spec);
  sector_erase_command = spec[0].sectorEraseCommand;
  program_page_command = spec[0].programPageCommand;

  return 0;
}

int flash_disconnect(void)
{
  return fl_disconnect();
}

int flash_locate_upgrade_slot(unsigned &address)
{
  fl_BootImageInfo info;

  int ret = fl_getFactoryImage(info);
  if (ret != 0)
    return ret;

  ret = fl_getNextBootImage(info);
  if (ret == 0) {
    address = info.startAddress;
  }
  else {
    // rounding up to whole sectors as per fl_initImageWriteState
    address = info.startAddress + info.size;
    address = fl_getSectorAddress(fl_getSectorAtOrAfter(address)); // sector aligned
  }

  return 0;
}

int flash_locate_data_upgrade_slot(unsigned &address)
{
  fl_DataImageInfo info;

  int ret = fl_getFactoryDataImage(info);
  if (ret != 0)
    return ret;

  if (fl_getNextDataImage(info) == 0) {
    address = info.startAddress;
  }
  else {
    address = info.startAddress + info.size;
    address = fl_getSectorAddress(fl_getSectorAtOrAfter(address));
  }

  return 0;
}

int flash_is_upgrade_slot_valid(bool &valid)
{
  fl_BootImageInfo info;

  int ret = fl_getFactoryImage(info);
  if (ret != 0)
    return ret;

  valid = (fl_getNextBootImage(info) == 0);

  return 0;
}

int flash_is_data_upgrade_slot_valid(bool &valid)
{
  fl_DataImageInfo info;

  int ret = fl_getFactoryDataImage(info);
  if (ret != 0)
    return ret;

  valid = (fl_getNextDataImage(info) == 0);

  return 0;
}

bool flash_is_first_whole_page_in_sector(unsigned address)
{
  int page_size = fl_getPageSize();

  if (address < page_size)
    return true;

  return fl_getSectorContaining(address - page_size) !=
         fl_getSectorContaining(address);
}

int flash_erase_sector_async(unsigned address)
{
  if (fl_setWritability(1) != 0)
    return 1;

  fl_int_eraseSector(sector_erase_command, address);

  return 0;
}

bool flash_is_sector_erased(unsigned address)
{
  unsigned page_address = address;
  int page_size = fl_getPageSize();
  while (fl_getSectorContaining(page_address) == fl_getSectorAtOrAfter(address)) {
    char page[QUADFLASHLIB_MAX_PAGE_SIZE];
    fl_readPage(page_address, page);
    for (int i = 0; i < page_size; i++) {
      if (page[i] != 0xFF)
        return false;
    }
    page_address += page_size;
  }
  return true;
}

int flash_write_page_async(unsigned address, const char page[])
{
  int page_size = fl_getPageSize();

  if (fl_setWritability(1) != 0)
    return 1;

  fl_int_write(program_page_command, address, page, page_size);

  return 0;
}

int flash_verify_page(unsigned address, const char page[])
{
  int page_size = fl_getPageSize();
  char verify[QUADFLASHLIB_MAX_PAGE_SIZE];

  fl_readPage(address, verify);
  return safememcmp(verify, page, page_size);
}

bool flash_is_busy(void)
{
  return fl_getBusyStatus() != 0;
}

int flash_set_write_disable(void)
{
  return fl_setWritability(0);
}
