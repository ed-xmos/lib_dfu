// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "fifo.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

int fifo_init(struct fifo *fifo, uint8_t *buffer, int size) {
  if (fifo == NULL || buffer == NULL || size <= 0) {
    return FIFO_BAD_PARAM;
  }
  fifo->buffer = buffer;
  fifo->max = size;
  fifo->head = 0;
  fifo->tail = 0;
  fifo->count = 0;
  return FIFO_OK;
}

int fifo_enqueue(struct fifo *fifo, uint8_t data) {
  if (fifo == NULL) {
    return FIFO_BAD_PARAM;
  } else if (fifo->count == fifo->max) {
    return FIFO_FULL;
  }

  fifo->buffer[fifo->head] = data;
  fifo->head += 1;
  if (fifo->head == fifo->max) {
    fifo->head = 0;
  }

  fifo->count++;

  return FIFO_OK;
}

struct fifo_status_data fifo_dequeue(struct fifo *fifo) {
  struct fifo_status_data result = {FIFO_ERROR, 0};
  if (fifo == NULL) {
    result.status = FIFO_BAD_PARAM;
    return result;
  } else if (fifo_is_empty(fifo)) {
    result.status = FIFO_EMPTY;
    return result;
  }

  result.data = fifo->buffer[fifo->tail];
  fifo->tail += 1;
  if (fifo->tail == fifo->max) {
    fifo->tail = 0;
  }

  fifo->count--;

  result.status = FIFO_OK;
  return result;
}

int fifo_is_empty(struct fifo *fifo) {
  if (fifo == NULL) {
    return FIFO_BAD_PARAM;
  }
  return (fifo->count == 0);
}

int fifo_is_full(struct fifo *fifo) {
  if (fifo == NULL) {
    return FIFO_BAD_PARAM;
  }
  return (fifo->count == fifo->max);
}

int fifo_size(struct fifo *fifo) {
  if (fifo == NULL) {
    return FIFO_BAD_PARAM;
  }
  return fifo->count;
}

/* Block operations */
static int f_block_enqueue_head_space(struct fifo *fifo) {
  if (fifo_is_full(fifo)) {
    return 0;
  } else if (fifo->head >= fifo->tail) {
    return fifo->max - fifo->head;
  } else {
    return fifo->tail - fifo->head;
  }
}

static int f_block_dequeue_tail_count(struct fifo *fifo) {
  if (fifo_is_empty(fifo)) {
    return 0;
  } else if (fifo->head <= fifo->tail) {
    return fifo->max - fifo->tail;
  } else {
    return fifo->head - fifo->tail;
  }
}

int fifo_block_enqueue(struct fifo *fifo, const uint8_t data[], int length) {
  if (fifo == NULL || data == NULL || length <= 0) {
    return FIFO_BAD_PARAM;
  }

  int space = fifo->max - fifo->count;
  if (length > space) {
    return FIFO_FULL;
  }

  int head_space = f_block_enqueue_head_space(fifo);
  if (length <= head_space) {
    memcpy(&fifo->buffer[fifo->head], data, (size_t)length);
    fifo->head += length;
    if (fifo->head >= fifo->max) {
      fifo->head -= fifo->max;
    }
    fifo->count += length;
  } else {
    memcpy(&fifo->buffer[fifo->head], data, (size_t)head_space);
    fifo->head = 0;
    fifo->count += head_space;

    int remaining = length - head_space;
    memcpy(&fifo->buffer[fifo->head], &data[head_space], (size_t)remaining);
    fifo->head += remaining;
    fifo->count += remaining;
  }

  return FIFO_OK;
}

int fifo_block_dequeue(struct fifo *fifo, uint8_t data[], int length) {
  if (fifo == NULL || data == NULL || length <= 0) {
    return FIFO_BAD_PARAM;
  }

  if (length > fifo->count) {
    return FIFO_EMPTY;
  }

  int tail_count = f_block_dequeue_tail_count(fifo);
  if (length <= tail_count) {
    memcpy(data, &fifo->buffer[fifo->tail], (size_t)length);
    fifo->tail += length;
    if (fifo->tail >= fifo->max) {
      fifo->tail -= fifo->max;
    }
    fifo->count -= length;
  } else {
    memcpy(data, &fifo->buffer[fifo->tail], (size_t)tail_count);
    fifo->tail = 0;
    fifo->count -= tail_count;

    int remaining = length - tail_count;
    memcpy(&data[tail_count], &fifo->buffer[fifo->tail], (size_t)remaining);
    fifo->tail += remaining;
    fifo->count -= remaining;
  }

  return FIFO_OK;
}
