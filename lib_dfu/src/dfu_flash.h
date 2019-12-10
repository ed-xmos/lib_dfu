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

int flash_prepare_image_write(REFERENCE_PARAM(fl_BootImageInfo, preceding));
int flash_finalise_image_write(void);

int flash_begin_page_write(const char page[], size_t page_size_bytes);
bool flash_has_page_write_completed(void);

#endif
