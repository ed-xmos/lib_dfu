// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef __options_h__
#define __options_h__

#include <stdbool.h>

extern bool verbose;

struct options {
  unsigned regular_sector_size;
  const char *factory_file_name;
  unsigned upgrade_comp_version;
  const char *upgrade_file_name;
  const char *out_file_name;
  bool bad_factory_crc;
  bool bad_upgrade_crc;
};

struct options parse_command_line(int argc, char **argv);

#endif
