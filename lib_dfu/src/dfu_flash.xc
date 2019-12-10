// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stddef.h>
#include <quadflash.h>

#define _Bool int
#include <stdbool.h>

#include "dfu_flash.h"

int flash_connect(fl_QSPIPorts &ports, const fl_QuadDeviceSpec spec[1])
{
  return fl_connectToDevice(ports, spec, 1);
}

int flash_disconnect(void)
{
  return fl_disconnect();
}

int flash_prepare_image_write(fl_BootImageInfo &preceding)
{
  fl_BootImageInfo upgrade;
  int ret;

  ret = fl_getFactoryImage(preceding);
  if (ret != 0)
    return ret;

  if (fl_getNextBootImage(upgrade) == 0)
    preceding = upgrade;

  return 0;
}

int flash_begin_page_write(const char page[], size_t page_size_bytes)
{
  return 0;
}

bool flash_has_page_write_completed(void)
{
  return true;
}

int flash_finalise_image_write(void)
{
  // quad-flash library has an end write-image call
  // that normally just sets write disable
  return fl_endWriteImage();
}
