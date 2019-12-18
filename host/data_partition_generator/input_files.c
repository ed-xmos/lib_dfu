// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stdio.h>
#include <stdlib.h>
#include "options.h"
#include "input_files.h"

struct input_files open_input_files(const struct options *options)
{
  struct input_files files;

  if (options->factory_file_name != NULL) {
    files.factory = fopen(options->factory_file_name, "rb");
    if (files.factory == NULL) {
      fprintf(stderr, "problem opening factory file %s\n", options->factory_file_name);
      exit(1);
    }
  }
  else {
    files.factory = NULL;
  }

  if (options->upgrade_file_name != NULL) {
    files.upgrade = fopen(options->upgrade_file_name, "rb");
    if (files.upgrade == NULL) {
      fprintf(stderr, "problem opening upgrade file %s\n", options->upgrade_file_name);
      exit(1);
    }
  }
  else {
    files.upgrade = NULL;
  }

  return files;
}

void close_input_files(struct input_files *files)
{
  if (files->factory != NULL) {
    fclose(files->factory);
    files->factory = NULL;
  }

  if (files->upgrade != NULL) {
    fclose(files->upgrade);
    files->upgrade = NULL;
  }
}
