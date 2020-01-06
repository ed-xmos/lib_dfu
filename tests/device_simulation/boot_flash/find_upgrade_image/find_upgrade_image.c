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

#define XFLASH_TAG 0x0FF51DE
#define FACTORY_PAGE_CRC 0x16A8334D
#define FACTORY_BYTE_SIZE 22052

const unsigned factory_start = 1418; // word address
const unsigned upgrade_start = 8192;

const unsigned upgrade_page_crc = 0xD27CAF74;

const unsigned factory_page[] = {
  XFLASH_TAG, FACTORY_PAGE_CRC, 0x00000000, 0x00000040,
  FACTORY_BYTE_SIZE, 0x00000000, 0x00005624, 0x00000002,
  0x00000000, 0x00000000, 0x00000000, 0x00000034,
  0x0000003C, 0x00000054, 0x000001D6, 0x000007AC,
  0x80020002, 0x0000139A, 0x00005614, 0x80030002,
  0x00000004, 0x7740F000, 0x918C7F40, 0x64806040,
  0xDB12F000, 0xF00037FB, 0x37EBDB1D, 0x6F81F000,
  0x7744F000, 0x54425401, 0xF0005483, 0x918CDB13,
  0xF0006840, 0xF0006C82, 0xF001D03D, 0x68406001,
  0x6C83F000, 0xD036F000, 0xD038F000, 0x6AC6F000,
  0xEACFEAC8, 0xD006F000, 0x5F425F01, 0x5F835FC4,
  0x000077C0, 0x7F8217FF, 0x6C05F000, 0x17FF6844,
  0xD017F000, 0xD025F000, 0x58BFF000, 0x17FFA6D1,
  0x17FFC802, 0x17FF4FF0, 0x6004F000, 0x6040F000,
  0x17FFA6DD, 0xD023F000, 0x5840F001, 0x17FF6802,
  0x17FFC804, 0x17FF4FE0, 0x600CF000, 0xD083F000
};

unsigned DeviceAccess_Streamed_CRC(unsigned word_address,
                                   unsigned num_words,
                                   unsigned expected_crc)
{
  const unsigned page_size_words = 64;
  assert(num_words == page_size_words - 3);

  if (word_address == factory_start + 3)
    assert(expected_crc == FACTORY_PAGE_CRC);
  else if (word_address == upgrade_start + 3)
    assert(expected_crc == upgrade_page_crc);
  else
    assert(0);

  return 0;
}

unsigned DeviceAccess_ReadWord(unsigned word_address)
{
  if (word_address == 0) { // stage 2 loader word size
    return factory_start - 2;
  }
  else if (word_address >= factory_start && word_address < upgrade_start) {
    unsigned word_offset = word_address - factory_start;
    if (word_offset < 64)
      return factory_page[word_offset];
  }
  else if (word_address >= upgrade_start) {
    unsigned word_offset = word_address - upgrade_start;
    if (word_offset == 0)
      return XFLASH_TAG;
    else if (word_offset == 1)
      return upgrade_page_crc;
  }

  assert(0);
  return -1;
}

unsigned DEVICE_SECTOR_WORD_SIZE = 1024; // IS25LQ016B

int main(void)
{
  int ret;
  unsigned factory = -1, upgrade = -1;

  ret = flash_find_factory_image(&factory);
  assert(ret == 0);
  debug_printf("%d\n", factory);

  ret = flash_find_upgrade_image(&upgrade, factory);
  assert(ret == 0);

  debug_printf("%d\n", upgrade);
  assert(upgrade == upgrade_start * 4);

  printstr("PASS\n");
  return 0;
}
