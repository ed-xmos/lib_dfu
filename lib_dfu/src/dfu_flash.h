// Copyright (c) 2020, XMOS Ltd, All rights reserved
#ifndef __dfu_flash_h__
#define __dfu_flash_h__

#define _Bool int
#include <stdbool.h>

int flash_locate_boot_upgrade_slot(unsigned &address);

int flash_locate_data_upgrade_slot(unsigned &address);

int flash_erase_sector_async(unsigned address);

bool flash_is_busy(void);

bool flash_is_first_whole_page_in_sector(unsigned address);

bool flash_is_sector_erased(unsigned address);

int flash_set_write_disable(void);

int flash_write_page_async(unsigned address, const char page[]);

int flash_verify_page(unsigned address, const char page[]);

#endif
