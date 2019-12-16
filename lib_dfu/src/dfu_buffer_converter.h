// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef __dfu_buffer_converter_h__
#define __dfu_buffer_converter_h__

#include <xccompat.h>

/* Essentially a small FIFO with variable input and output element size
 *
 * The name buffer converter represents its typical use for converting
 * DFU blocks to flash pages. Flash page is normally 256 bytes. Some USB DFU
 * implementations use DFU blocks of 32 bytes, so smaller than flash page.
 * Others might use as much as 512 bytes, and it is useful to use the same
 * routines for converting from one size to another.
 *
 * In addition to push and pull operations, there is a padded pull. This is
 * a pull of any amount of data up to the given size, padded with zeroes.
 * Typically this will be used to remove a partial page that may have
 * accumulated as a result of partial blocks.
 */

#define BUFFER_CONVERTER_QUEUE_SIZE_BYTES 512
#define BUFFER_CONVERTER_PADDING_BYTE 0x00

struct buffer_converter {
  int wp; // write pointer
  int rp; // read pointer
  int fullness;
  int capacity;
  char storage[BUFFER_CONVERTER_QUEUE_SIZE_BYTES];
};

void buffer_converter_reset(REFERENCE_PARAM(struct buffer_converter, obj));

int buffer_converter_push(REFERENCE_PARAM(struct buffer_converter, obj),
                          const char data[], int data_size_bytes);

int buffer_converter_pull(REFERENCE_PARAM(struct buffer_converter, obj),
                          char data[], int data_size_bytes);

int buffer_converter_padded_pull(REFERENCE_PARAM(struct buffer_converter, obj),
                                 char data[], int data_size_max_bytes);

#endif
