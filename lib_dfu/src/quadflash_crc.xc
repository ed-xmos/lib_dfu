// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <print.h>
#include "quadflash_crc.h"

unsigned crc_init(void)
{
  return 0xFFFFFFFF;
}

void crc_step(unsigned &crc, unsigned word)
{
  crc32(crc, word, 0xEDB88320);
}

unsigned crc_finish(unsigned crc)
{
  return ~crc;
}
