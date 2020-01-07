// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

bool verbose = false;

size_t pad_to_flash_sector(size_t section_size, unsigned sector_size);

int main(void)
{
  size_t ret;

  ret = pad_to_flash_sector(1, 4096);
  assert(ret == 4096);

  ret = pad_to_flash_sector(4095, 4096);
  assert(ret == 4096);

  ret = pad_to_flash_sector(4096, 4096);
  assert(ret == 4096);

  ret = pad_to_flash_sector(4097, 4096);
  assert(ret == 8192);

  ret = pad_to_flash_sector(8191, 4096);
  assert(ret == 8192);

  ret = pad_to_flash_sector(8192, 4096);
  assert(ret == 8192);

  printf("PASS\n");
  return 0;
}
