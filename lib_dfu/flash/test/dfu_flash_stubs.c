// Copyright 2020-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <assert.h>
#include <stdbool.h>
#include <xassert.h>

#include "dfu.h"
#include "dfu_flash.h"
#include "dfu_flash_result.h"

enum flash_locate_boot_upgrade_slot_result flash_locate_boot_upgrade_slot(unsigned *address) {
  UNUSED(address);

  assert(0);
  return FLASH_LOCATE_BOOT_UPGRADE_SLOT_GET_FACTORY_IMAGE_FAILED;
}

enum flash_locate_data_upgrade_slot_result flash_locate_data_upgrade_slot(unsigned *address) {
  UNUSED(address);

  assert(0);
  return FLASH_LOCATE_DATA_UPGRADE_SLOT_GET_FACTORY_DATA_IMAGE_NO_CHECKSUM_FAILED;
}

enum flash_status flash_write_page(const unsigned char page[], int length) {
  UNUSED(page);
  UNUSED(length);
  assert(0);
  return DFU_FLASH_WRITE_ERROR;
}

int flash_is_busy(void) {
  assert(0);
  return false;
}

int flash_is_first_whole_page_in_sector(unsigned address) {
  (void)address;

  assert(0);
  return false;
}

int flash_is_sector_erased(unsigned address) {
  (void)address;

  assert(0);
  return false;
}

enum flash_set_write_disable_result flash_set_write_disable(void) {
  assert(0);
  return FLASH_SET_WRITE_DISABLE_ERROR;
}

bool flash_verify_page(unsigned address, const char page[]) {
  (void)address;
  UNUSED(page);

  assert(0);
  return false;
}

int flash_get_page_size(void) {
  assert(0);
  return -1;
}

int flash_get_data_partition_base(void) {
  assert(0);
  return -1;
}

int flash_get_size(void) {
  assert(0);
  return -1;
}
