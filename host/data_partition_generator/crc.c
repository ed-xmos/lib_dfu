// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stdio.h>
#include <stdbool.h>
#include "crc.h"

unsigned crc_init(void)
{
  return 0xFFFFFFFF;
}

unsigned crc_step(unsigned crc, unsigned word)
{
  const unsigned poly = 0xEDB88320;
  for (int b = 0; b < 32; b++) {
    bool xor_bit = (crc & 1) == 1;
    crc = (crc >> 1) | ((word & 1) << 31);
    word >>= 1;
    if (xor_bit)
      crc ^= poly;
  }
  return crc;
}

unsigned crc_finish(unsigned crc)
{
  return ~crc;
}
