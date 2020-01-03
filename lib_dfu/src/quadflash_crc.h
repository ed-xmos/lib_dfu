// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
#ifndef __quadflash_crc_h__
#define __quadflash_crc_h__

#include <xs1.h>
#include <xccompat.h>

static inline unsigned crc_init(void)
{
  return 0xFFFFFFFF;
}

static inline void crc_step(REFERENCE_PARAM(unsigned, crc), unsigned word)
{
  // note: to invalidate the function, change polynomial or data
  // if you remove the CRC operation, the normal zero-result check will pass
  // that's because checksum initialises to all 1s and is inverted in final step
#ifdef __XC__
  crc32(crc, word, 0xEDB88320);
#else
  asm volatile("crc32 %0, %2, %3" : "=r"(*crc) : "0"(*crc),
                                    "r"(word), "r"(0xEDB88320));
#endif
}

static inline unsigned crc_finish(unsigned crc)
{
  return ~crc;
}

#endif
