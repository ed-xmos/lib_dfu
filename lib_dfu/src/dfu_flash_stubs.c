// Copyright (c) 2020, XMOS Ltd, All rights reserved
#ifdef DFU_FLASH_UNIT_TEST
#include <assert.h>
#include <stdbool.h>
#include "dfu_flash.h"

__attribute__((weak))
int flash_locate_boot_upgrade_slot(unsigned *address);

int flash_locate_boot_upgrade_slot(unsigned *address)
{
  assert(0);
  return -1;
}

__attribute__((weak))
int flash_locate_data_upgrade_slot(unsigned *address);

int flash_locate_data_upgrade_slot(unsigned *address)
{
  assert(0);
  return -1;
}

__attribute__((weak))
int flash_erase_sector_async(unsigned address);

int flash_erase_sector_async(unsigned address)
{
  assert(0);
  return -1;
}

__attribute__((weak))
int flash_is_busy(void);

int flash_is_busy(void)
{
  assert(0);
  return false;
}

__attribute__((weak))
int flash_is_first_whole_page_in_sector(unsigned address);

int flash_is_first_whole_page_in_sector(unsigned address)
{
  assert(0);
  return false;
}

__attribute__((weak))
int flash_is_sector_erased(unsigned address);

int flash_is_sector_erased(unsigned address)
{
  assert(0);
  return false;
}

__attribute__((weak))
int flash_set_write_disable(void);

int flash_set_write_disable(void)
{
  assert(0);
  return -1;
}

__attribute__((weak))
int flash_write_page_async(unsigned address, const char page[]);

int flash_write_page_async(unsigned address, const char page[])
{
  assert(0);
  return -1;
}

__attribute__((weak))
int flash_verify_page(unsigned address, const char page[]);

int flash_verify_page(unsigned address, const char page[])
{
  assert(0);
  return -1;
}

__attribute__((weak))
int flash_get_page_size(void);

int flash_get_page_size(void)
{
  assert(0);
  return -1;
}

__attribute__((weak))
int flash_get_data_partition_base(void);

int flash_get_data_partition_base(void)
{
  assert(0);
  return -1;
}

__attribute__((weak))
int flash_get_size(void);

int flash_get_size(void)
{
  assert(0);
  return -1;
}

#endif
