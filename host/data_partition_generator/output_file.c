// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stdio.h>
#include <stdlib.h>
#include "images.h"
#include "options.h"
#include "output_file.h"

struct output_file open_output_file(const struct options *options)
{
  struct output_file file;

  if (options->out_file_name != NULL) {
    file.output = fopen(options->out_file_name, "wb");
    if (file.output == NULL) {
      fprintf(stderr, "problem opening output file %s\n", options->out_file_name);
      exit(1);
    }
  }
  else {
    file.output = NULL;
  }

  return file;
}

void close_output_file(struct output_file *file)
{
  if (file->output != NULL) {
    fclose(file->output);
    file->output = NULL;
  }
}

void write_images(struct output_file *file, const struct images *images)
{
  fwrite(images->hardware_build, 1, images->hardware_build_size, file->output);
  fwrite(images->factory, 1, images->factory_size, file->output);
  fwrite(images->upgrade, 1, images->upgrade_size, file->output);
}
