// Copyright (c) 2020, XMOS Ltd, All rights reserved
//
// Remember that dependency tree building under xcommon-based waf often means
// a clean build is required
//
#ifndef __boot_h__
#define __boot_h__

#include <xs1.h>

#define _Bool int
#include <stdbool.h>

#include "boot_flash.h"

unsigned factory_address = 0;
unsigned upgrade_address = 0;

bool upgrade_offered = false;
bool data_upgrade_valid = false;

int error_code = 0;

void init(void)
{
  if (flash_find_factory_image(&factory_address) != 0)
    error_code |= 0x01;
  else if (flash_find_upgrade_image(&upgrade_address, factory_address) != 0)
    error_code |= 0x02;
}

int checkCandidateImageVersion(int xflash_version)
{
  return 1; // interested in all available images
}

void recordCandidateImage(int xflash_version, unsigned address)
{
  if (address == upgrade_address) {
    if (xflash_version == 0)
      error_code |= 0x10;

    if (flash_is_data_upgrade_slot_valid(&data_upgrade_valid) != 0)
      error_code |= 0x20;

    upgrade_offered = true;
  }
}

unsigned reportSelectedImage(void)
{
  if (upgrade_offered && data_upgrade_valid)
    return upgrade_address;
  else
    return factory_address;
}

#endif
