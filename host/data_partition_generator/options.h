// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef __options_h__
#define __options_h__

#include <stdbool.h>

struct options {
  bool verbose;
  unsigned regular_sector_size;
  const char *factory_file_name;
  unsigned upgrade_comp_version;
  const char *upgrade_file_name;
  const char *out_file_name;
};

struct options parse_command_line(int argc, char **argv);

#endif
