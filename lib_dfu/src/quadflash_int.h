// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef __quadflash_int_h__
#define __quadflash_int_h__

#include <quadflash.h>
#include <quadflashlib.h> // the lower level functions

void fl_saveSpecPointer(const fl_QuadDeviceSpec spec[1]);

void fl_int_eraseSector(unsigned char cmd, unsigned int sectorAddress);

int fl_getSectorContaining(unsigned address);
int fl_getSectorAtOrAfter(unsigned address);
int fl_getSectorEndAddress(int sectorNum);

#endif
