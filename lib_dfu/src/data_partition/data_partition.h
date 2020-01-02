// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
#ifndef __data_partition_h__
#define __data_partition_h__

#include <stdint.h>

struct data_partition_hardware_build {
  uint32_t tag;
  uint32_t data;
  uint32_t checksum;
};

#define DATA_PARTITION_HARDWARE_BUILD_TAG 0xDAB8DAB8

struct data_partition_image_header {
  uint32_t tag;
  uint32_t data_size_words;
  uint32_t comp_version;
  uint32_t checksum;
};

#define DATA_PARTITION_IMAGE_TAG 0xDA16DA16

#endif
