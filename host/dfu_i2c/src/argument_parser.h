// Copyright 2020-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef __argument_parser_h__
#define __argument_parser_h__

#ifdef __XC__
#define _Bool int
#endif
#include <stdbool.h>
#include "device_id.h"

// 0xFFFF means ignore ID checks in DFU suffix verification
#define DFU_SUFFIX_IGNORE_ID 0xFFFF

#define I2C_ADDRESS_DEFAULT 0x2C
#define BLOCK_SIZE_DEFAULT 128

extern bool quiet;

struct options {
  enum {
    UNKNOWN = 0,
    WRITE_UPGRADE,
    DETACH_AND_BUS_RESET,
    REBOOT,
    REVERT_FACTORY
  } operation;
  const char *arguments[2];
  struct device_id device_id;
  unsigned block_size;
};

struct options parse_arguments(int argc, char **argv);

#endif
