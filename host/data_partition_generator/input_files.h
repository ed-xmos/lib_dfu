// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef __input_files_h__
#define __input_files_h__

#include <stdio.h>
#include "options.h"

struct input_files {
  FILE *factory;
  FILE *upgrade;
};

struct input_files open_input_files(const struct options *options);
void close_input_files(struct input_files *files);

#endif
