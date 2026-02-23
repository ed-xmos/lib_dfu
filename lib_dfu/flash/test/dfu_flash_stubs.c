// Copyright 2020-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "dfu_flash.h"

#include <assert.h>
#include <stdint.h>
#include <xassert.h>

#include "dfu.h"
#if defined (DFU_ENABLE) && (DFU_ENABLE == 0)

static int32_t device_open;

enum flash_status flash_init(void) __attribute__((weak));
enum flash_status flash_init(void) {
  device_open = 1;
  return DFU_FLASH_OK;
}

enum flash_status flash_deinit(void) __attribute__((weak));
enum flash_status flash_deinit(void) {
  if (device_open) {
    device_open = 0;
  }
  return DFU_FLASH_OK;
}

int32_t flash_is_connected(void) __attribute__((weak));
int32_t flash_is_connected(void) {
  return device_open;
}

struct flash_data_status flash_get_image_size_from_buffer(const uint8_t buf[], int32_t length) __attribute__((weak));
struct flash_data_status flash_get_image_size_from_buffer(const uint8_t buf[], int32_t length){
  (void)buf;
  (void)length;

  // Intended to override with conf.h in test
  struct flash_data_status result = { DFU_FLASH_BAD_PARAM, FLASH_MAX_UPGRADE_SIZE };
  return result;
}

enum flash_status flash_erase_sector_async(int32_t erase_size) __attribute__((weak));
enum flash_status flash_erase_sector_async(int32_t erase_size) {
  (void)erase_size;

  assert(0);
  return DFU_FLASH_ERASE_ERROR;
}

enum flash_status flash_write_page(const uint8_t *page, int32_t length) __attribute__((weak));
enum flash_status flash_write_page(const uint8_t *page, int32_t length) {
  (void)page;
  (void)length;
  assert(0);
  return DFU_FLASH_WRITE_ERROR;
}

enum flash_status flash_finalise_write() __attribute__((weak));
enum flash_status flash_finalise_write() {
  assert(0);
  return DFU_FLASH_WRITE_ERROR;
}

enum flash_status flash_read_page(uint8_t *data, int32_t length) __attribute__((weak));
enum flash_status flash_read_page(uint8_t *data, int32_t length) {
  (void)data;
  (void)length;

  assert(0);
  return DFU_FLASH_READ_ERROR;
}

struct flash_data_status flash_start_read() __attribute__((weak));
struct flash_data_status flash_start_read() {
  assert(0);

  struct flash_data_status result = { DFU_FLASH_READ_ERROR, FLASH_MAX_UPGRADE_SIZE };
  return result;
}

int32_t flash_is_busy(void) __attribute__((weak));
int32_t flash_is_busy(void) {
  return 0;
}

int32_t flash_get_page_size(void) __attribute__((weak));
int32_t flash_get_page_size(void) {
  return DFU_FLASH_PAGE_SIZE_BYTES;
}

int32_t flash_get_sector_size(void) __attribute__((weak));
int32_t flash_get_sector_size(void) {
  return (4 * 1024);
}

int32_t flash_get_size(void) __attribute__((weak));
int32_t flash_get_size(void) {
  return 512 * (4 * 1024); // 512 sectors of 4KB each, i.e. 2MB total
}

int32_t flash_is_suitable(void) __attribute__((weak));
int32_t flash_is_suitable(void)
{
  return 1;
}

#endif
