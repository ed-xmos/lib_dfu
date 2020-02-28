// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <platform.h>
#include <stdio.h>
#include <print.h>
#include <string.h>

#define _Bool int
#include <stdbool.h>

#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

#define DEBUG_UNIT TEST
#define DEBUG_PRINT_ENABLE_TEST 0
#include "debug_print.h"

#include "dfu.h"

unsigned boot_slot_address = 0;
unsigned data_slot_address = 0;

unsigned first_test_address = 0;
unsigned erase_sector_address = 0;

bool flash_is_first_whole_page_in_sector(unsigned address)
{
  first_test_address = address;
  return true;
}

int flash_erase_sector_async(unsigned address)
{
  erase_sector_address = address;
  return 0;
}

bool flash_is_busy(void)
{
  return false;
}

int flash_locate_boot_upgrade_slot(unsigned &address)
{
  address = boot_slot_address;
  return 0;
}

int flash_locate_data_upgrade_slot(unsigned &address)
{
  address = data_slot_address;
  return 0;
}

bool flash_is_sector_erased(unsigned address)
{
  return false;
}

int flash_set_write_disable(void)
{
  return 0;
}

int flash_write_page_async(unsigned address, const char page[])
{
  return 0;
}

int flash_verify_page(unsigned address, const char page[])
{
  return 0;
}

int flash_get_page_size(void)
{
  return 0;
}

int main(unsigned argc, char * unsafe argv[argc])
{
  struct dfu_getstatus getstatus;
  enum dfu_state state;
  char block[DFU_BLOCK_SIZE_MAX_BYTES];
  int block_num = -1;
  unsigned expected = ~0;
  int ret;

  assert(argc == 5);

  unsafe {
    sscanf(argv[1], "%u", &boot_slot_address);
    sscanf(argv[2], "%u", &data_slot_address);
    sscanf(argv[3], "0x%x", &block_num);
    sscanf(argv[4], "%u", &expected);
  }

  ret = dfu_locate_upgrade_slots();
  assert(ret == 0);

  state = dfu_getstate();
  assert(state == APP_IDLE);

  dfu_detach();
  state = dfu_getstate();
  assert(state == APP_DETACH);

  dfu_bus_reset();
  state = dfu_getstate();
  assert(state == DFU_IDLE);

  dfu_dnload(block_num, sizeof(block), block);
  
  state = dfu_getstate();
  assert(state == DFU_DNLOAD_SYNC);

  getstatus = dfu_getstatus();
  assert(getstatus.status == DFU_OK);

  assert(erase_sector_address == expected);
  assert(first_test_address == expected);

  printstr("PASS\n");
  return 0;
}
