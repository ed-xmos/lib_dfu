// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef __quadflash_data_partition_h__
#define __quadflash_data_partition_h__

#include <xccompat.h>
#include <quadflash.h>

typedef fl_BootImageInfo fl_DataImageInfo;

void fl_saveSpecPointer(const fl_QuadDeviceSpec spec[1]);

int fl_getFactoryDataImage(REFERENCE_PARAM(fl_DataImageInfo, dataImageInfo));

int fl_getNextDataImage(REFERENCE_PARAM(fl_DataImageInfo, dataImageInfo));

#endif
