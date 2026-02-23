// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef __labels_h__
#define __labels_h__

#include "dfu_commands.h"
#include "dfu_types.h"

const char *command_str(enum dfu_command command);
const char *state_str(enum dfu_state state);
const char *status_str(enum dfu_status status);

#endif
