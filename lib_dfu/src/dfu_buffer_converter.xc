// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include "dfu_buffer_converter.h"

void buffer_converter_reset(struct buffer_converter &obj)
{
  obj.wp = 0;
  obj.rp = 0;
  obj.fullness = 0;
  obj.capacity = BUFFER_CONVERTER_QUEUE_SIZE_BYTES;
}

int buffer_converter_push(struct buffer_converter &obj,
                          const char data[], int data_size_bytes)
{
  if (obj.fullness + data_size_bytes > obj.capacity)
    return 1;

  for (int i = 0; i < data_size_bytes; i++) {
    obj.storage[obj.wp] = data[i];
    obj.wp = (obj.wp + 1) % BUFFER_CONVERTER_QUEUE_SIZE_BYTES;
  }

  obj.fullness += data_size_bytes;
  return 0;
}

int buffer_converter_pull(struct buffer_converter &obj,
                          char data[], int data_size_bytes)
{
  if (obj.fullness - data_size_bytes < 0)
    return 1;

  for (int i = 0; i < data_size_bytes; i++) {
    data[i] = obj.storage[obj.rp];
    obj.rp = (obj.rp + 1) % BUFFER_CONVERTER_QUEUE_SIZE_BYTES;
  }

  obj.fullness -= data_size_bytes;
  return 0;
}
