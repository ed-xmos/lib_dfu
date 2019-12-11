// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef __dfu_flash_h__
#define __dfu_flash_h__

#include <stddef.h>
#include <xccompat.h>
#include <quadflash.h>

#define _Bool int
#include <stdbool.h>

int flash_connect(REFERENCE_PARAM(fl_QSPIPorts, ports),
                  const fl_QuadDeviceSpec spec[1]);

int flash_disconnect();

int flash_locate_upgrade_slot(REFERENCE_PARAM(unsigned, address));

int flash_is_upgrade_slot_valid(REFERENCE_PARAM(bool, valid));

int flash_set_write_disable(void);

bool flash_is_first_whole_page_in_sector(unsigned address);

int flash_erase_sector_async(unsigned address);

bool flash_is_sector_erased(unsigned address);

int flash_write_page_async(unsigned address, const char page[]);

int flash_verify_page(unsigned address, const char page[]);

bool flash_is_busy(void);

#endif
