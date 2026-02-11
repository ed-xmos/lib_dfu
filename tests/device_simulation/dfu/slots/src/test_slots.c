// Copyright 2020-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <xs1.h>
#include <platform.h>
#include <stdio.h>
#include <print.h>
#include <string.h>
#include <unity.h>

#define _Bool int
#include <stdbool.h>

#define DEBUG_UNIT TEST
#define DEBUG_PRINT_ENABLE_TEST 0
#include "debug_print.h"

#include "dfu.h"
#include "dfu_flash.h"

static int erase_requested_size = 0;

enum flash_status flash_erase_sector_async(int erase_size)
{
  erase_requested_size = erase_size;
  return DFU_FLASH_OK;
}

enum flash_status flash_write_page(const unsigned char page[], int length)
{
  (void)page;
  (void)length;
  return DFU_FLASH_OK;
}

enum flash_status flash_finalise_write() {
  return DFU_FLASH_OK;
}

enum flash_status flash_start_read() {
  return DFU_FLASH_OK;
}

enum flash_status flash_read_page(unsigned char* data, int length)
{
  (void)data;
  (void)length;
  return DFU_FLASH_OK;
}

bool flash_is_busy(void)
{
  return false;
}

int flash_get_page_size(void) {
  return 256;
}

int flash_get_sector_size(void) {
  return 4096;
}

int flash_get_size(void) {
  return 8 * 1024 * 1024;
}

/* DEPRECATED Flash functions */
#include "dfu_flash_result.h"

bool flash_is_first_whole_page_in_sector(unsigned address) {
  return (address % (unsigned)flash_get_sector_size()) == 0;
}

bool flash_is_sector_erased(unsigned address) {
  (void)address;
  return true;
}

enum flash_locate_boot_upgrade_slot_result flash_locate_boot_upgrade_slot(unsigned* address) {
  *address = 0;
  return FLASH_LOCATE_BOOT_UPGRADE_SLOT_SUCCESS;
}

enum flash_locate_data_upgrade_slot_result flash_locate_data_upgrade_slot(unsigned* address) {
  *address = 16 * 1024;
  return FLASH_LOCATE_DATA_UPGRADE_SLOT_SUCCESS;
}

enum flash_set_write_disable_result flash_set_write_disable(void) {
  return FLASH_SET_WRITE_DISABLE_SUCCESS;
}

bool flash_verify_page(unsigned address, const char page[]) {
  (void)address;
  (void)page;
  return true;
}

int flash_get_data_partition_base(void) {
  return 32 * 1024;
}

void flash_cmd_read_page(unsigned char *data) {
  (void)data;
  return;
}


void test_slots(void)
{
  struct dfu_getstatus getstatus;
  enum dfu_state state;
  char block[DFU_TRANSFER_SIZE_BYTES];
  int expected = FLASH_MAX_UPGRADE_SIZE;
  int ret;

  ret = dfu_locate_upgrade_slots();
  TEST_ASSERT_EQUAL_INT(0, ret);

  state = dfu_getstate();
  TEST_ASSERT_EQUAL_INT(APP_IDLE, state);

  dfu_detach();
  state = dfu_getstate();
  TEST_ASSERT_EQUAL_INT(APP_DETACH, state);

  dfu_bus_reset();
  state = dfu_getstate();
  TEST_ASSERT_EQUAL_INT(DFU_IDLE, state);

  unsigned short block_num = 0;
  dfu_dnload(block_num, sizeof(block), block);
  
  state = dfu_getstate();
  TEST_ASSERT_EQUAL_INT(DFU_DNLOAD_SYNC, state);

  getstatus = dfu_getstatus();
  TEST_ASSERT_EQUAL_INT(DFU_OK, getstatus.status);

  TEST_ASSERT_EQUAL_INT(expected, erase_requested_size);
}
