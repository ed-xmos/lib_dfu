// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "data_image.h"
#include "input_files.h"
#include "descriptions.h"

struct descriptions parse_descriptions(const struct input_files *files)
{
  struct descriptions d;
  memset(&d, 0, sizeof(struct descriptions));

  if (files->factory != NULL) {
    d.hardware_build.build_word = 0x123;
    d.factory.tlv_data_size_words = 1;
    d.factory.tlv_data = malloc(sizeof(int));
    d.factory.tlv_data[0] = CUSTOMER_VERSION;
    d.factory.tlv_data[1] = 2;
    d.factory.tlv_data[2] = 0x00;
    d.factory.tlv_data[3] = 0x01;
    d.factory.comp_version = 513;
  }

  if (files->upgrade != NULL) {
    d.upgrade.tlv_data_size_words = 1;
    d.upgrade.tlv_data = malloc(sizeof(int));
    d.upgrade.tlv_data[0] = CUSTOMER_VERSION;
    d.upgrade.tlv_data[1] = 2;
    d.upgrade.tlv_data[2] = 0x01;
    d.upgrade.tlv_data[3] = 0x01;
    d.upgrade.comp_version = 514;
  }

  return d;
}

void free_tlv_data(struct descriptions *descriptions)
{
  if (descriptions->factory.tlv_data != NULL) {
    free(descriptions->factory.tlv_data);
    descriptions->factory.tlv_data = NULL;
  }

  if (descriptions->upgrade.tlv_data != NULL) {
    free(descriptions->upgrade.tlv_data);
    descriptions->upgrade.tlv_data = NULL;
  }
}
