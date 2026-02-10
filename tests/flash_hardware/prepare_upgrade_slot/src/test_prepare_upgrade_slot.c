// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unity.h>
#include <xcore/hwtimer.h>

#include "dfu.h"
#include "dfu_flash.h"

/* Main args */
uint8_t* upgrade_mem = NULL;
uint32_t erase_timing_threshold_ms = 0;
uint32_t write_timing_threshold_ms = 0;

static int upgrade_size = 0;
static hwtimer_t prepare_timer;

void setUp() { prepare_timer = hwtimer_alloc(); }

void tearDown() {
  flash_cmd_deinit();
  hwtimer_free(prepare_timer);
}

int write(hwtimer_t runtime, uint8_t* mem, int length) {

  uint32_t start_runtime = hwtimer_get_time(runtime);
  uint32_t max_runtime = start_runtime + (60UL * XS1_TIMER_HZ);  // 60s
  uint32_t running;
  enum flash_status wr_status = DFU_FLASH_OK;
  int read_total = 0;

  uint8_t *page = mem;
  int page_size = flash_get_page_size();
  do {
    wr_status = flash_write_page(page, page_size);
    page += page_size;
    read_total += page_size;

    running = hwtimer_get_time(runtime);
  } while ((wr_status == DFU_FLASH_OK) && (read_total < length) && !hwtimer_time_after(running, max_runtime));

  printf("Read total %d\n", read_total);
  printf("Write time: %0.3fs\n", (float)(running - start_runtime) / (float)XS1_TIMER_HZ);  // Typically ~80ms seconds
  TEST_ASSERT_LESS_THAN_UINT32((write_timing_threshold_ms * XS1_TIMER_KHZ), (running - start_runtime));
  return wr_status;
}

int read(hwtimer_t runtime, uint8_t* mem, int length) {

  uint32_t start_runtime = hwtimer_get_time(runtime);
  uint32_t max_runtime = start_runtime + (60UL * XS1_TIMER_HZ);  // 60s
  uint32_t running;
  enum flash_status rd_status = DFU_FLASH_OK;
  int read_total = 0;
  int match = 1;

  uint8_t *page = mem;
  uint8_t verify[DFU_FLASH_PAGE_SIZE_BYTES] = {0};
  int page_size = flash_get_page_size();
  TEST_ASSERT_EQUAL(DFU_FLASH_PAGE_SIZE_BYTES, page_size);
  enum flash_status prep_status = flash_start_read();
  TEST_ASSERT_EQUAL(DFU_FLASH_OK, prep_status);
  do {
    rd_status = flash_read_page(verify, page_size);
    match &= (memcmp(page, verify, (unsigned)page_size) == 0);
    page += page_size;
    read_total += page_size;

    running = hwtimer_get_time(runtime);
  } while ((rd_status == DFU_FLASH_OK) && match && (read_total < length) && !hwtimer_time_after(running, max_runtime));

  printf("Read status %d\n", rd_status);
  printf("Read total %d\n", read_total);
  printf("Read time: %0.3fs\n", (float)(running - start_runtime) / (float)XS1_TIMER_HZ);  // Typically ~80ms seconds
  TEST_ASSERT_TRUE(match);
  TEST_ASSERT_LESS_THAN_UINT32((write_timing_threshold_ms * XS1_TIMER_KHZ), (running - start_runtime));
  return rd_status;
}

// TODO - Ideally test with factory-only and upgrade image present
#include <stdio.h>

void test_dfu_image_analysis(void) {
  fl_BootImageInfo boot_image_info;
  int status = fl_getImageInfo(&boot_image_info, upgrade_mem);
  TEST_ASSERT_EQUAL(0, status);
  upgrade_size = (int)boot_image_info.size;
}

void test_dfu_flash_prepare_slot_reports_OK(void) {
  int status = flash_cmd_init();
  TEST_ASSERT_EQUAL(DFU_FLASH_OK, status);
  
  uint32_t start_runtime = hwtimer_get_time(prepare_timer);
  uint32_t max_runtime = start_runtime + (60UL * XS1_TIMER_HZ);  // 60s
  uint32_t running;
  int erase = DFU_FLASH_BUSY;

  do {
    erase = flash_erase_sector_async(0);
    hwtimer_delay(prepare_timer, 10UL * XS1_TIMER_KHZ);  // 10ms

    running = hwtimer_get_time(prepare_timer);
  } while (erase == DFU_FLASH_BUSY && !hwtimer_time_after(running, max_runtime));

  printf("Erase time: %0.2fs\n", (float)(running - start_runtime) / XS1_TIMER_HZ);  // Typically ~7 seconds
  TEST_ASSERT_GREATER_THAN_UINT32((4000 * XS1_TIMER_KHZ), (running - start_runtime));
  TEST_ASSERT_LESS_THAN_UINT32((erase_timing_threshold_ms * XS1_TIMER_KHZ), (running - start_runtime));
  TEST_ASSERT_EQUAL(DFU_FLASH_OK, erase);
}

void test_dfu_flash_write_reports_OK(void) {
  TEST_ASSERT_NOT_NULL(upgrade_mem);
  TEST_ASSERT_NOT_EQUAL(0, upgrade_size);

  int status = flash_cmd_init();
  TEST_ASSERT_EQUAL(DFU_FLASH_OK, status);

  int wr_status = write(prepare_timer, upgrade_mem, upgrade_size);

  enum flash_status final = flash_finalise_write();

  TEST_ASSERT_EQUAL(DFU_FLASH_OK, wr_status);
  TEST_ASSERT_EQUAL(DFU_FLASH_OK, final);
  // Upgrade image is now valid
}

void test_dfu_flash_verify_reports_OK(void) {
  TEST_ASSERT_NOT_NULL(upgrade_mem);
  TEST_ASSERT_NOT_EQUAL(0, upgrade_size);

  int status = flash_cmd_init();
  TEST_ASSERT_EQUAL(DFU_FLASH_OK, status);

  enum flash_status prep_status = flash_start_read();
  TEST_ASSERT_EQUAL(DFU_FLASH_OK, prep_status);

  int rd_status = read(prepare_timer, upgrade_mem, upgrade_size);

  TEST_ASSERT_EQUAL(DFU_FLASH_OK, rd_status);
}
