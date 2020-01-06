// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <platform.h>
#include <print.h>

#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

#define DEBUG_UNIT TEST
#define DEBUG_PRINT_ENABLE_TEST 0
#include "debug_print.h"

#include "boot_flash.h"

const unsigned factory_start = 1418; // word address
const unsigned factory_size = 20480;
const unsigned xflash_tag = 0x0FF51DE;
const unsigned page_crc = 0x16A8334D;
const unsigned boot_partition_size = 1048576;

unsigned DeviceAccess_Streamed_CRC(unsigned word_address,
                                   unsigned num_words,
                                   unsigned expected_crc)
{
  assert(word_address == factory_start + 3);
  assert(num_words == 61);
  assert(expected_crc == page_crc);
  return 0;
}

unsigned DeviceAccess_ReadWord(unsigned word_address)
{
  if (word_address == 0) { // stage 2 loader word size
    return factory_start - 2;
  }
  else if (word_address == 3) {
    return boot_partition_size;
  }
  else if (word_address >= factory_start) {
    unsigned word_offset = word_address - factory_start;
    if (word_offset == 0)
      return xflash_tag;
    else if (word_offset == 1)
      return page_crc;
    else if (word_offset == 4)
      return factory_size;
  }

  assert(0);
  return -1;
}

int main(void)
{
  int ret;
  unsigned address;

  ret = flash_find_factory_image(&address);
  assert(ret == 0);

  debug_printf("%d\n", address);
  assert(address == factory_start * 4);

  printstr("PASS\n");
  return 0;
}
