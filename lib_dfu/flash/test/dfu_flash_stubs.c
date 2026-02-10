// Copyright 2020-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <xassert.h>

#include "dfu.h"
#include "dfu_flash.h"

static int32_t device_open;

enum flash_status flash_cmd_init(void) {
  device_open = 1;
  return DFU_FLASH_OK;
}

enum flash_status flash_cmd_deinit(void) {
  if (device_open) {
    device_open = 0;
  }
  return DFU_FLASH_OK;
}

int32_t flash_is_connected(void) {
  return device_open;
}

struct flash_data_status flash_get_image_size_from_buffer(const uint8_t buf[], int32_t length){
  (void)buf;
  (void)length;

  struct flash_data_status result = { DFU_FLASH_BAD_PARAM, 0 };
  return result;
}

enum flash_status flash_erase_sector_async(int32_t erase_size) {
  (void)erase_size;

  assert(0);
  return DFU_FLASH_ERASE_ERROR;
}

enum flash_status flash_write_page(const uint8_t *page, int32_t length) {
  UNUSED(page);
  UNUSED(length);
  assert(0);
  return DFU_FLASH_WRITE_ERROR;
}

enum flash_status flash_finalise_write() {
  assert(0);
  return DFU_FLASH_WRITE_ERROR;
}

enum flash_status flash_read_page(uint8_t *data, int32_t length) {
  UNUSED(data);
  UNUSED(length);
  assert(0);
  return DFU_FLASH_READ_ERROR;
}

struct flash_data_status flash_start_read() {
  assert(0);
  struct flash_data_status result = { DFU_FLASH_READ_ERROR, 0 };
  return result;
}

bool flash_is_busy(void) {
  assert(0);
  return false;
}

int32_t flash_get_page_size(void) {
  assert(0);
  return -1;
}

int32_t flash_get_data_partition_base(void) {
  assert(0);
  return -1;
}

int32_t flash_get_sector_size(void) {
  assert(0);
  return -1;
}

int32_t flash_get_size(void) {
  assert(0);
  return -1;
}

bool flash_is_suitable(void)
{
  return true;
}
