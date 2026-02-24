// Copyright 2020-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef __hal_h__
#define __hal_h__

#include <stddef.h>
#include "dfu_host_commands.h"
#include "device_id.h"

int hal_connect(struct device_id device_id);

int hal_read_command(int command,
                     unsigned char *payload, size_t num_bytes);

int hal_write_command(int command,
                      const unsigned char *payload, size_t num_bytes);

int hal_reboot(void);

int hal_disconnect(void);

#endif
