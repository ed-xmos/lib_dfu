// Copyright 2020-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#ifdef __xcore__
#include <string.h>
#else
#include <memory.h>
#include <time.h>
#endif
#include <stdint.h>

// byte order portability
#ifdef _WIN32
#define htole16(x) (x) // Windows little endian only
#define le32toh(x) (x)
#elif __APPLE__
#include <libkern/OSByteOrder.h>
#define htole16(x) OSSwapHostToLittleInt16(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)
#elif __xcore__
#define htole16(x) (x) // XCore is little endian
#define le32toh(x) (x)
#else
#include <endian.h>
#endif

#include "labels.h"
#include "dfu_utils.h"
#include "hal.h"
#include "input_reader.h"
#include "operations.h"

extern bool quiet;

static int check_state(enum dfu_state expected)
{
  uint8_t payload[DFU_GET_STATE_PAYLOAD_SIZE_BYTES];

  if (hal_read_command(DFU_GETSTATE, payload, sizeof(payload)) != 0) {
    return 1;
  }
  // convert from hard little endian order after deserialisation
  enum dfu_state state = (enum dfu_state)le32toh(payload[DFU_GETSTATE_INDEX]);

  if (state == STATE_DFU_ERROR) {
    PRINT_ERROR("Device in dfu ERROR state\n");
    
    uint8_t payload_status[DFU_GET_STATUS_PAYLOAD_SIZE_BYTES];

    struct dfu_getstatus getstatus = { 0 };
    if (hal_read_command(DFU_GETSTATUS, payload_status, sizeof(payload_status)) == 0) {
      getstatus.status = (enum dfu_status)le32toh(payload_status[DFU_GETSTATUS_STATUS_INDEX]);
      PRINT_ERROR("Status %s\n", status_str(getstatus.status));
    }

    PRINT_ERROR("Send CLRSTATUS to attempt recovery\n");
    hal_write_command(DFU_CLRSTATUS, NULL, 0);
    return 2;
  }

  if (state != expected) {
    PRINT_ERROR("Device state is %s (%d), expected %s (%d)\n", state_str(state), state, state_str(expected), expected);
    return 3;
  }

  return 0;
}

static int check_status(struct dfu_getstatus *getstatus)
{
  uint8_t payload[DFU_GET_STATUS_PAYLOAD_SIZE_BYTES];
  if (hal_read_command(DFU_GETSTATUS, payload, sizeof(payload)) != 0) {
    return 1;
  }

  // convert from hard little endian order after deserialization
  getstatus->state = le32toh(payload[DFU_GETSTATUS_STATE_INDEX]);
  getstatus->status = le32toh(payload[DFU_GETSTATUS_STATUS_INDEX]);
  getstatus->poll_timeout_msec = le32toh(payload[DFU_GETSTATUS_POLL_TIMEOUT_INDEX]) & ((1 << (DFU_GETSTATUS_POLL_TIMEOUT_BYTES * 8)) - 1);

  if (getstatus->status != DFU_OK) {
    PRINT_ERROR("Status was %s when %s expected\n", status_str(getstatus->status), status_str(DFU_OK));

    PRINT_ERROR("State %s (%d)\n", state_str(getstatus->state), getstatus->state);

    PRINT_ERROR("Send CLRSTATUS to attempt recovery\n");
    hal_write_command(DFU_CLRSTATUS, NULL, 0);
    return 2;
  }

  static unsigned last_timeout = 0;
  if (!quiet) {
    printf("poll timeout %u msec\n", getstatus->poll_timeout_msec);
  } else if (getstatus->poll_timeout_msec != last_timeout) {
    last_timeout = getstatus->poll_timeout_msec;
    printf("new poll timeout %u msec\n", getstatus->poll_timeout_msec);
  }

  return 0;
}

int detach_and_bus_reset(void)
{
  if (!quiet) {
    printf("detach and bus reset\n");
  }

  if (check_state(STATE_APP_IDLE) == 0) {
    if (hal_write_command(DFU_DETACH, NULL, 0) != 0) {
      return 2;
    }

    if (check_state(STATE_APP_DETACH) != 0) {
      return 3;
    }

    if (hal_write_command(XMOS_BUS_RESET, NULL, 0) != 0) {
      return 4;
    }

  } else {
    if (hal_write_command(DFU_ABORT, NULL, 0) != 0) {
      return 6;
    }
  }

  if (check_state(STATE_DFU_IDLE) != 0) {
    return 7;
  }

  if (!quiet) {
    printf("detach and bus reset successful\n");
  }

  return 0;
}

static int download_file(const unsigned char *bytes, size_t length, unsigned block_size, unsigned short marker)
{
  size_t byte_count = 0;
  unsigned block_count = 0;
  struct dfu_getstatus getstatus;

  if (!quiet) {
    printf("start download of %d bytes, block size %d, marker 0x%X\n", (int)length, block_size, marker); // size_t different in xCORE unit test
  }

  while (byte_count < length) {
    size_t block_bytes = block_size;
    if (length - byte_count < block_size)
      block_bytes = length - byte_count;

    printf("download block %u, %d bytes\n", block_count, (int)block_bytes); // size_t different in xCORE unit test

    if (hal_write_command(DFU_DNLOAD, bytes + byte_count, block_bytes) != 0) {
      return 1;
    }

    do {
      if (check_status(&getstatus) != 0) {
        return 2;
      }

      sleep_milliseconds(getstatus.poll_timeout_msec);
    } while (getstatus.state == STATE_DFU_DOWNLOAD_BUSY);

    if (check_state(STATE_DFU_DOWNLOAD_IDLE) != 0) {
      return 3;
    }

    block_count++;
    byte_count += block_bytes;
  }

  if (hal_write_command(DFU_DNLOAD, NULL, 0) != 0) {
    return 4;
  }

  do {
    if (check_status(&getstatus) != 0) {
      return 5;
    }

    sleep_milliseconds(getstatus.poll_timeout_msec);
  } while (getstatus.state == STATE_DFU_MANIFEST);

  if (check_state(STATE_DFU_IDLE) != 0) {
    return 6;
  }

  return 0;
}

int write_upgrade(struct inputs inputs, unsigned block_size)
{
  if (!quiet) {
    printf("write upgrade %d boot bytes\n", (int)inputs.boot.length);
  }

  if (detach_and_bus_reset() != 0) {
    return 1;
  }

  if (download_file(inputs.boot.bytes, inputs.boot.length, block_size, 0) != 0) {
    return 2;
  }

  if (!quiet) {
    printf("write upgrade successful\n");
  }

  return 0;
}

int override_spispec(struct inputs inputs)
{
  if (!quiet) {
    printf("override spispec (%d bytes)\n", (int)inputs.spispec.length);
  }

  (void)inputs;
  PRINT_ERROR("override-spispec is not supported by this lib_dfu host build\n");
  return 1;
}
