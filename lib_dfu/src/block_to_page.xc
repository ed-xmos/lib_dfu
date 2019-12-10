// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include "block_to_page.h"

void block_to_page_reset(struct block_to_page &buffer)
{
  buffer.wp = 0;
  buffer.rp = 0;
  buffer.fullness = 0;
  buffer.capacity = BLOCK_TO_PAGE_BUFFER_SIZE_BYTES;
}

int block_to_page_push(struct block_to_page &buffer,
                       const char block[], int block_size_bytes)
{
  if (buffer.fullness + block_size_bytes > buffer.capacity)
    return 1;

  for (int i = 0; i < block_size_bytes; i++) {
    buffer.storage[buffer.wp] = block[i];
    buffer.wp = (buffer.wp + 1) % BLOCK_TO_PAGE_BUFFER_SIZE_BYTES;
  }

  buffer.fullness += block_size_bytes;
  return 0;
}

int block_to_page_pull(struct block_to_page &buffer,
                       char page[], int page_size_bytes)
{
  if (buffer.fullness - page_size_bytes < 0)
    return 1;

  for (int i = 0; i < page_size_bytes; i++) {
    page[i] = buffer.storage[buffer.rp];
    buffer.rp = (buffer.rp + 1) % BLOCK_TO_PAGE_BUFFER_SIZE_BYTES;
  }

  buffer.fullness -= page_size_bytes;
  return 0;
}
