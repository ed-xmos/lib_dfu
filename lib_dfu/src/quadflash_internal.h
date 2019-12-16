// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef __quadflash_internal_h__
#define __quadflash_internal_h__

#include <quadflash.h>
#include <quadflashlib.h> // the lower level functions

#define QUADFLASHLIB_MAX_PAGE_SIZE 256

void fl_int_eraseSector(unsigned char cmd, unsigned int sectorAddress);

#ifdef __XC__
void fl_int_write(unsigned char cmd,
                  unsigned int pageAddress, 
                  const unsigned char data[num_bytes],
                  unsigned int num_bytes);
#else
void fl_int_write(unsigned char cmd,
                  unsigned int pageAddress, 
                  const unsigned char data[],
                  unsigned int num_bytes);
#endif

int fl_getSectorContaining(unsigned address);
int fl_getSectorAtOrAfter(unsigned address);
int fl_getSectorEndAddress(int sectorNum);

#endif
