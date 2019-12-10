// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <print.h>

#define DEBUG_UNIT TEST
#define DEBUG_PRINT_ENABLE_TEST 1
#include "debug_print.h"

#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

#include "dfu_buffer_converter.h"

void random_sequence(char seq[], int length)
{
  for (int i = 0; i < length; i++) {
    unsigned x;
    crc32(x, -1, 0xEB31D82E);
    seq[i] = x;
  }
}

int test(int block_size, int page_size, int repeats)
{
  char generated[BUFFER_CONVERTER_QUEUE_SIZE_BYTES];
  char block[BUFFER_CONVERTER_QUEUE_SIZE_BYTES];
  char page[BUFFER_CONVERTER_QUEUE_SIZE_BYTES];
  int result = 0;
  int ret;

  struct buffer_converter converter;
  buffer_converter_reset(converter);

  debug_printf("+ %d-%d (%dx)\n", block_size, page_size, repeats);

  if (block_size > page_size) {
    int multiplier = block_size / page_size;
    for (int k = 0; k < repeats; k++) {
      random_sequence(generated, block_size);
      for (int i = 0; i < block_size; i++) {
        block[i] = generated[i];
      }
      ret = buffer_converter_push(converter, block, block_size);
      assert(ret == 0);
      for (int j = 0; j < multiplier; j++) {
        ret = buffer_converter_pull(converter, page, page_size);
        assert(ret == 0);
        for (int i = 0; i < page_size; i++) {
          if (page[i] != generated[page_size * j + i]) {
            result |= 1;
            debug_printf("run %d/%d page %d offset %d mismatch: %02X %02X\n",
              k, repeats, j, i, page[i], generated[page_size * j + i]);
          }
        }
      }
      ret = buffer_converter_pull(converter, page, page_size);
      assert(ret == 1);
    }
  }
  else {
    int multiplier = page_size / block_size;
    for (int k = 0; k < repeats; k++) {
      random_sequence(generated, page_size);
      for (int j = 0; j < multiplier; j++) {
        for (int i = 0; i < block_size; i++) {
          block[i] = generated[block_size * j + i];
        }
        ret = buffer_converter_push(converter, block, block_size);
        assert(ret == 0);
      }
      ret = buffer_converter_pull(converter, page, page_size);
      assert(ret == 0);
      for (int i = 0; i < page_size; i++) {
        if (page[i] != generated[i]) {
          result != 1;
          debug_printf("run %d/%d page offset %d mismatch: %02X %02X\n",
            k, repeats, i, page[i], generated[i]);
        }
      }
      ret = buffer_converter_pull(converter, page, page_size);
      assert(ret == 1);
    }
  }

  return result;
}

int main(void)
{
  int bases[] = {16, 32, 64};
  int multipliers[] = {1, 2, 4, 8};
  int repeats = 4;

  int result = 0;

  for (int i = 0; i < sizeof(bases) / sizeof(int); i++) {
    for (int j = 0; j < sizeof(multipliers) / sizeof(int); j++) {
      for (int k = 1; k <= repeats; k++) {
        result |= test(bases[i], bases[i] * multipliers[j], k);
        if (multipliers[j] > 1)
          result |= test(bases[i] * multipliers[j], bases[i], k);
      }
    }
  }

  if (result == 0)
    printstr("PASS\n");

  return result;
}
