// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
#ifndef __checksum_h__
#define __checksum_h__

#include <stdint.h>
#include <stddef.h>
#include "data_partition_structures.h"

uint32_t checksum_hardware_build_section(const struct data_partition_hardware_build *build);

uint32_t checksum_image(const struct data_partition_image_header *header,
                        const uint8_t *data, size_t data_num_words);

#endif
