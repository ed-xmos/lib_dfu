// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stdio.h>
#include <syscall.h>
#include "xassert.h"
#include "dfu.h"

int main(unsigned argc, char * unsafe argv[argc])
{
  int ret;
  int outfile;
  char data[DFU_DATA_MAX];

  unsafe {
    outfile = _open((const char*)argv[1],
                    O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE); 
  }
  assert(outfile > -1);

  ret = dfu_do_read_command(DFU_GETSTATE, data);
  assert(ret == 0);

  ret = _write(outfile, data, DFU_DATA_MAX);
  assert(ret == DFU_DATA_MAX);

  _close(outfile);

  return 0;
}
