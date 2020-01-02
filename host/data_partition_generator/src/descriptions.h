// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
#ifndef __descriptions_h__
#define __descriptions_h__

#include <stdint.h>
#include "input_files.h"

struct descriptions {
  struct {
    uint32_t build_word;
  } hardware_build;
  struct {
    size_t tlv_data_size_words;
    uint32_t comp_version;
    uint8_t *tlv_data;
  } factory, upgrade;
};

struct descriptions parse_descriptions(const struct input_files *files);
void free_tlv_data(struct descriptions *descriptions);

#endif
