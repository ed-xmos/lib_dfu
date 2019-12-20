// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef __quadflash_crc_h__
#define __quadflash_crc_h__

#include <xccompat.h>

unsigned crc_init(void);
void crc_step(REFERENCE_PARAM(unsigned, crc), unsigned word);
unsigned crc_finish(unsigned crc);

#endif
