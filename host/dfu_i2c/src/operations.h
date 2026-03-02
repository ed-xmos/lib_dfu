// Copyright 2020-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef __operations_h__
#define __operations_h__

#include <stdbool.h>
#include "input_reader.h"

int write_upgrade(struct inputs inputs, unsigned block_size);

int detach_and_bus_reset(void);

#endif
