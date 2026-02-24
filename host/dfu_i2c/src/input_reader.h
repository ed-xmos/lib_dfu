// Copyright 2020-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef __input_reader_h__
#define __input_reader_h__

#include <stddef.h>
#include "device_id.h"

struct inputs {
  struct {
    unsigned char *bytes;
    size_t length;
  } boot, spispec;
};

struct inputs read_write_upgrade_inputs(const char *boot_file_name,
                                        struct device_id device_id);

struct inputs read_override_spispec_input(const char *spispec_file_name);

void cleanup_inputs(struct inputs *inputs);

#endif
