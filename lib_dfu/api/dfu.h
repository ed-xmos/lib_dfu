// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef __dfu_h__
#define __dfu_h__

#define DFU_DATA_MAX 32

enum dfu_command {
  DFU_GETSTATE
};

int dfu_do_read_command(enum dfu_command command, char data[DFU_DATA_MAX]);

int dfu_do_write_command(enum dfu_command command, const char data[DFU_DATA_MAX]);

#endif
