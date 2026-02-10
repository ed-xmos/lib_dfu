// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef FIFO_H
#define FIFO_H

#include <stdint.h>
#include <xccompat.h>

#if defined __XC__ || defined __cplusplus
extern "C" {
#endif

enum fifo_status {
  FIFO_OK = 0,
  FIFO_ERROR = -1,
  FIFO_FULL = -2,
  FIFO_EMPTY = -3,
  FIFO_BAD_PARAM = -4
};

struct fifo {
  uint8_t* buffer;
  int head;
  int tail;
  int max;  // of the buffer
  int count;
};

struct fifo_status_data {
  enum fifo_status status;
  uint8_t data;
};

#if defined __XC__ || defined __cplusplus
}
#endif

/** Initialization of fifo
 * \param fifo Pointer to fifo structure
 * \param buffer Pointer to buffer to be used by fifo
 * \param size Size of the buffer
 * \retval FIFO_OK on success
 * \retval FIFO_BAD_PARAM if bad parameters, null pointers or size <= 0
 */
int fifo_init(REFERENCE_PARAM(struct fifo, fifo), uint8_t buffer[], int size);

/** Enqueue byte into fifo
 * \param fifo Pointer to fifo structure
 * \param data Byte to enqueue
 * \retval FIFO_OK on success
 * \retval FIFO_FULL if fifo is full
 * \retval FIFO_BAD_PARAM if fifo pointer is null
 */
int fifo_enqueue(REFERENCE_PARAM(struct fifo, fifo), uint8_t data);

/** Dequeue byte from fifo
 * \param fifo Pointer to fifo structure
 * \return fifo_status_data structure containing status and dequeued byte
 * \retval FIFO_OK on success, data contains dequeued byte
 * \retval FIFO_EMPTY if fifo is empty
 * \retval FIFO_BAD_PARAM if fifo pointer is null
 */
struct fifo_status_data fifo_dequeue(REFERENCE_PARAM(struct fifo, fifo));

/** Check if fifo is empty
 * \param fifo Pointer to fifo structure
 * \retval 1 if empty
 * \retval 0 if not empty
 * \retval FIFO_BAD_PARAM if fifo pointer is null
 */
int fifo_is_empty(REFERENCE_PARAM(struct fifo, fifo));

/** Check if fifo is full
 * \param fifo Pointer to fifo structure
 * \retval 1 if full
 * \retval 0 if not full
 * \retval FIFO_BAD_PARAM if fifo pointer is null
 */
int fifo_is_full(REFERENCE_PARAM(struct fifo, fifo));

/** Get number of bytes stored in fifo
 * \param fifo Pointer to fifo structure
 * \retval count number of bytes on success
 * \retval FIFO_BAD_PARAM if fifo pointer is null
 */
int fifo_size(REFERENCE_PARAM(struct fifo, fifo));

/* Block operations */
/** Enqueue block of bytes into fifo
 * \param fifo Pointer to fifo structure
 * \param data Pointer to data block to enqueue
 * \param length Number of bytes to enqueue
 * \retval FIFO_OK on success
 * \retval FIFO_BAD_PARAM if fifo or data pointer is null, or length <= 0
 * \retval FIFO_FULL if fifo does not have enough space, fifo does not perform best-effort so if space is insufficient
 * no data is enqueued, \retval FIFO_BAD_PARAM if fifo or data pointer is null
 */
int fifo_block_enqueue(REFERENCE_PARAM(struct fifo, fifo), const uint8_t data[], int length);

/** Dequeue block of bytes from fifo
 * \param fifo Pointer to fifo structure
 * \param data Pointer to buffer to store dequeued data
 * \param length Number of bytes to dequeue
 * \retval FIFO_OK on success
 * \retval FIFO_BAD_PARAM if fifo or data pointer is null, or length <= 0
 * \retval FIFO_EMPTY if fifo does not have enough data, fifo does not perform best-effort so if insufficient data is
 * available no data is dequeued, \retval FIFO_BAD_PARAM if fifo or data pointer is null
 */
int fifo_block_dequeue(REFERENCE_PARAM(struct fifo, fifo), uint8_t data[], int length);

#endif  // FIFO_H
