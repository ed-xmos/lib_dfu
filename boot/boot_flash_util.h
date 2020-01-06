// Copyright (c) 2020, XMOS Ltd, All rights reserved
#ifndef __boot_flash_util_h__
#define __boot_flash_util_h__

#define BOOT_PARTITION_SIZE_WORD_OFFSET 3 // see BOOT_PARTITION_OFFSET in quadflashlib.c

#include "sqi_access.h"

// copied from SQI library (DeviceAccess_SQI.xc)
static inline unsigned char nibble_swap_byte(unsigned char c)
{
  return ((c & 0x0F) << 4) | ((c & 0xF0) >> 4);
}

static inline unsigned nibble_swap_word(unsigned int x)
{
  unsigned int y = 0;
  y = (nibble_swap_byte((x & 0xFF000000) >> 24) << 24);
  y |= (nibble_swap_byte((x & 0x00FF0000) >> 16) << 16);
  y |= (nibble_swap_byte((x & 0x0000FF00) >> 8) << 8);
  y |= nibble_swap_byte(x & 0x000000FF);
  return y;
}

static inline unsigned get_boot_partition_size(void)
{
  return DeviceAccess_ReadWord(BOOT_PARTITION_SIZE_WORD_OFFSET);
}

static inline unsigned sector_address_at_or_after(unsigned address)
{
  unsigned sector_size = DEVICE_SECTOR_WORD_SIZE * 4;
  if (address % sector_size == 0)
    return address; // at
  else
    return (address / sector_size + 1) * sector_size; // or after
}

static inline unsigned crc_init(void)
{
  return 0xFFFFFFFF;
}

static inline void crc_step(unsigned *crc, unsigned word)
{
  asm volatile("crc32 %0, %2, %3" : "=r"(*crc) : "0"(*crc),
                                    "r"(word), "r"(0xEDB88320));
}

static inline unsigned crc_finish(unsigned crc)
{
  return ~crc;
}

#endif
