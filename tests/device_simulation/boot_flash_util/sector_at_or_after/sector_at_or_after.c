// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <print.h>

#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

#include "boot_flash_util.h"

unsigned DeviceAccess_ReadWord(unsigned word_address)
{
  assert(0);
  return -1;
}

unsigned DEVICE_SECTOR_WORD_SIZE = 1024; // IS25LQ016B

int main(void)
{
  assert(sector_address_at_or_after(0) == 0);

  assert(sector_address_at_or_after(1) == 4096);
  assert(sector_address_at_or_after(1024) == 4096);
  assert(sector_address_at_or_after(4095) == 4096);
  assert(sector_address_at_or_after(4096) == 4096);

  assert(sector_address_at_or_after(4097) == 8192);
  assert(sector_address_at_or_after(5120) == 8192);
  assert(sector_address_at_or_after(8191) == 8192);
  assert(sector_address_at_or_after(8192) == 8192);

  printstr("PASS\n");
  return 0;
}
