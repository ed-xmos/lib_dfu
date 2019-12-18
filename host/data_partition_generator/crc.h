// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef __crc_h__
#define __crc_h__

unsigned crc_init(void);
unsigned crc_step(unsigned crc, unsigned word);
unsigned crc_finish(unsigned crc);

#endif
