// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef __checksum_h__
#define __checksum_h__

#include <stdint.h>
#include <stddef.h>
#include "data_partition.h"

void checksum_hardware_build_section(struct data_partition_hardware_build *build);

void checksum_image(struct data_partition_image_header *header,
                    const uint8_t *data, size_t data_num_words);

#endif
