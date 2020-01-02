// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "options.h"
#include "input_files.h"
#include "output_file.h"
#include "descriptions.h"
#include "images.h"

int main(int argc, char **argv)
{
  struct options options = parse_command_line(argc, argv);

  struct input_files input_files = open_input_files(options.factory_file_name,
                                                    options.upgrade_file_name);

  struct output_file output_file = open_output_file(options.out_file_name);

  struct descriptions descriptions = parse_descriptions(&input_files);

  if (options.upgrade_file_name != NULL &&
      (descriptions.upgrade.comp_version !=
      options.upgrade_comp_version)) {
    fprintf(stderr,
"error: command line specified upgrade compatibility version 0x%X\n\
but upgrade description file specified 0x%X\n",
            options.upgrade_comp_version, descriptions.upgrade.comp_version);
    exit(1);
  }

  struct images images;

  render_descriptions(&images, &options, &descriptions);

  write_images(&output_file, &images);

  free_tlv_data(&descriptions);
  close_input_files(&input_files);
  close_output_file(&output_file);

  return 0;
}
