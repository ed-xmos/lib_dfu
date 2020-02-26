// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
#ifndef __dfu_h__
#define __dfu_h__

#include <stddef.h>
#include <quadflash.h>

#define _Bool int
#include <stdbool.h>

#include "dfu_types.h"

enum dfu_state dfu_getstate(void);

struct dfu_getstatus dfu_getstatus(void);

void dfu_clrstatus(void);

void dfu_detach(void);

void dfu_bus_reset(void);

void dfu_timeout_detach(void);

void dfu_dnload(unsigned short block_num, size_t block_size_bytes,
                const char block[DFU_BLOCK_SIZE_MAX_BYTES]);

int dfu_get_error_info(void);

int dfu_locate_upgrade_slots(void);

bool dfu_is_flash_suitable(const fl_QuadDeviceSpec spec[1]);

#endif
