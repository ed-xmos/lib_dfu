// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef __block_to_page_h__
#define __block_to_page_h__

#define BLOCK_TO_PAGE_BUFFER_SIZE_BYTES 512

struct block_to_page {
  int wp; // write pointer
  int rp; // read pointer
  int fullness;
  int capacity;
  char storage[BLOCK_TO_PAGE_BUFFER_SIZE_BYTES];
};

void block_to_page_reset(struct block_to_page &buffer);

int block_to_page_push(struct block_to_page &buffer,
                       const char block[], int block_size_bytes);

int block_to_page_pull(struct block_to_page &buffer,
                       char page[], int page_size_bytes);

#endif
