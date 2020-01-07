// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
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
  assert(nibble_swap_byte(0x12) == 0x21);
  assert(nibble_swap_word(0xABCD1234) == 0xBADC2143);
  printstr("PASS\n");
  return 0;
}
