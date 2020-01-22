// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include "dfu_suffix.h"
#include "crc.h"

bool verbose = false;

int main(int argc, char **argv)
{
  bool correct_usage = false;
  unsigned vendor_id, product_id, bcd_device;
  if (argc == 4) {
    vendor_id = strtoul(argv[1], NULL, 0);
    product_id = strtoul(argv[2], NULL, 0);
    bcd_device = strtoul(argv[3], NULL, 0);
    if (vendor_id != 0 && product_id != 0 && bcd_device != 0)
      correct_usage = true;
  }
  if (!correct_usage) {
    fprintf(stderr, "\
usage: suffix_generator VENDOR_ID PRODUCT_ID BCD_DEVICE\n\
\n\
       arguments are non-zero 16bit hex values, eg 0x01AB\n\
       specify 0xFFFF if unused (eg BCD_DEVICE)\n");
    exit(1);
  }

  unsigned crc = crc_init();
  while (!feof(stdin)) {
    char buf[1024];
    size_t read = fread(buf, 1, sizeof(buf), stdin);
    for (int i = 0; i < read; i++) {
      crc_step(&crc, buf[i]);
    }
    if (fwrite(buf, 1, read, stdout) != read) {
      fprintf(stderr, "error: I/O write and read mismatch\n");
      exit(1);
    }
  }
  crc = crc_finish(crc);

  struct dfu_suffix suffix = {
    .crc = crc,
    .suffix_length = sizeof(struct dfu_suffix),
    .signature = DFU_SIGNATURE,
    .bcd_dfu = DFU_BCD,
    .vendor_id = vendor_id,
    .product_id = product_id,
    .bcd_device = bcd_device
  };
  char reversed[sizeof(struct dfu_suffix)];
  for (int i = 0; i < sizeof(reversed); i++) {
    reversed[i] = ((char*)&suffix)[sizeof(reversed) - 1 - i];
  }
  if (fwrite(reversed, sizeof(reversed), 1, stdout) != 1) {
    fprintf(stderr, "error: I/O write of suffix invalid return value\n");
    exit(1);
  }

  return 0;
}
