// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stdlib.h>
#include <quadflash.h>
#include "quadflash_data_partition.h"

static const fl_QuadDeviceSpec* g_flashAccess = NULL;

void fl_saveSpecPointer(const fl_QuadDeviceSpec spec[1])
{
  g_flashAccess = spec;
}
