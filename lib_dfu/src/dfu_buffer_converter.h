// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef __dfu_buffer_converter_h__
#define __dfu_buffer_converter_h__

#include <xccompat.h>

#define BUFFER_CONVERTER_QUEUE_SIZE_BYTES 512

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

#endif
