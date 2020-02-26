// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include "dfu_suffix.h"
#include "crc.h"
#include <string.h>

bool verbose = false;

int main(int argc, char **argv)
{
  bool correct_usage = false;
  unsigned vendor_id, product_id;
  char file_in[100];
  char file_out[100];

  if (argc == 5) {
    vendor_id = strtoul(argv[1], NULL, 0);
    product_id = strtoul(argv[2], NULL, 0);
    strcpy(file_in, argv[3]);
    strcpy(file_out, argv[4]);

    if (vendor_id != 0 && product_id != 0)
      correct_usage = true;
  }
  if (!correct_usage) {
    fprintf(stderr, "\
usage: dfu_suffix_generator VENDOR_ID PRODUCT_ID BIN_FILE_IN DFU_FILE_OUT\n\
\n\
       VENDOR_ID and PRODUCT_ID are non-zero 16bit hex values, eg 0x01AB\n\
       0xFFFF means do not verify this field\n\
       0 is invalid value\n");
    exit(1);
  }

  unsigned crc = crc_init();
  char buf[1024];
  FILE * file_in_stream = fopen(file_in, "r");
  FILE * file_out_stream = fopen(file_out, "w");
  size_t read = 0;
  if (file_in_stream && file_out_stream) {
    while ((read = fread(buf, 1, sizeof(buf), file_in_stream)) != 0) {
            
      for (int i = 0; i < read; i++) {
        crc_step(&crc, buf[i]);
      }
    
      if (fwrite(buf, 1, read, file_out_stream) != read) {
        fprintf(stderr, "error: I/O write and read mismatch\n");
        exit(1);
      }
    }
    fclose(file_in_stream);

    crc = crc_finish(crc);

    struct dfu_suffix suffix = {
      .crc = crc,
      .suffix_length = sizeof(struct dfu_suffix),
      .signature = DFU_SIGNATURE,
      .bcd_dfu = DFU_BCD,
      .vendor_id = vendor_id,
      .product_id = product_id,
      .bcd_device = 0xFFFF
    };
    char reversed[sizeof(struct dfu_suffix)];
    for (int i = 0; i < sizeof(reversed); i++) {
      reversed[i] = ((char*)&suffix)[sizeof(reversed) - 1 - i];
    }
    if (fwrite(reversed, sizeof(reversed), 1, file_out_stream) != 1) {
      fprintf(stderr, "error: I/O write of suffix invalid return value\n");
      exit(1);
    }
  }

  return 0;
}
