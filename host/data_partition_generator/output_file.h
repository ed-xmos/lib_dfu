// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef __output_file_h__
#define __output_file_h__

#include <stdio.h>
#include "images.h"
#include "options.h"

struct output_file {
  FILE *output;
};

struct output_file open_output_file(const struct options *options);
void close_output_file(struct output_file *file);

void write_images(struct output_file *file, const struct images *images);

#endif
