// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "data_image_defines.h"
#include "input_files.h"
#include "descriptions.h"

// hardcode values until JSON parsing is implemented
struct descriptions parse_descriptions(const struct input_files *files)
{
  struct descriptions d;
  memset(&d, 0, sizeof(struct descriptions));

  if (files->factory != NULL) {
    d.hardware_build.build_word = 0x123;
    d.factory.comp_version = 513;

    static const char tlv[] = {
      CUSTOMER_VERSION, 2, 0x00, 0x01,
      DATA_IMAGE_TYPE_STOP, 0, 0x00, 0x00
    };
    d.factory.tlv_data = (uint8_t*)tlv;
    d.factory.tlv_data_size_words = sizeof(tlv) / sizeof(int);
  }

  if (files->upgrade != NULL) {
    d.upgrade.comp_version = 514;

    static const char tlv[] = {
      CUSTOMER_VERSION, 2, 0x01, 0x01,
      DATA_IMAGE_TYPE_STOP, 0, 0x00, 0x00
    };
    d.upgrade.tlv_data = (uint8_t*)tlv;
    d.upgrade.tlv_data_size_words = sizeof(tlv) / sizeof(int);
  }

  return d;
}

void free_tlv_data(struct descriptions *descriptions)
{
  if (descriptions->factory.tlv_data != NULL) {
    descriptions->factory.tlv_data = NULL;
  }

  if (descriptions->upgrade.tlv_data != NULL) {
    descriptions->upgrade.tlv_data = NULL;
  }
}
