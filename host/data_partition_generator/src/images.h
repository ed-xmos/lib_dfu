// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
#ifndef __images_h__
#define __images_h__

#include <stdint.h>
#include <stddef.h>
#include "options.h"
#include "descriptions.h"

#define IMAGE_MAX 1048576

struct images {
  uint8_t hardware_build[IMAGE_MAX];
  size_t hardware_build_size;
  uint8_t factory[IMAGE_MAX];
  size_t factory_size;
  uint8_t upgrade[IMAGE_MAX];
  size_t upgrade_size;
};

void render_descriptions(struct images *images,
                         const struct options *options,
                         const struct descriptions *descriptions);

#endif
