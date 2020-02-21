// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
//
// The original intention is for those functions to be the ones used by the
// DFU implementation, and those only
//
#ifndef __dfu_flash_h__
#define __dfu_flash_h__

#include <xccompat.h>

#define _Bool int
#include <stdbool.h>

int flash_erase_sector_async(unsigned address);

int flash_get_page_size(void);

bool flash_is_busy(void);

bool flash_is_first_whole_page_in_sector(unsigned address);

bool flash_is_sector_erased(unsigned address);

int flash_locate_boot_upgrade_slot(REFERENCE_PARAM(unsigned, address));

int flash_locate_data_upgrade_slot(REFERENCE_PARAM(unsigned, address));

int flash_set_write_disable(void);

int flash_verify_page(unsigned address, const char page[]);

int flash_write_page_async(unsigned address, const char page[]);

#endif
