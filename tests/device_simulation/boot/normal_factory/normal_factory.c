// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <print.h>
#include <stdbool.h>

#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

#define DEBUG_UNIT TEST
#define DEBUG_PRINT_ENABLE_TEST 0
#include "debug_print.h"

#include "boot.h"

const unsigned factory_start = 5672;

int flash_find_factory_image(unsigned *address)
{
  *address = factory_start;
  return 0;
}

int flash_find_upgrade_image(unsigned *address, unsigned factory_start_arg)
{
  assert(factory_start_arg == factory_start);
  *address = 0;
  return 1;
}

int flash_is_data_upgrade_slot_valid(bool *valid)
{
  *valid = false;
  return 0;
}

int main(void)
{
  int ret;
  unsigned selected;

  init();

  ret = checkCandidateImageVersion(0);
  assert(ret == 1);

  recordCandidateImage(0, factory_start);

  selected = reportSelectedImage();
  assert(selected == factory_start);

  printstr("PASS\n");
  return 0;
}
