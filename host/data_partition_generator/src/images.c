// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "data_partition_structures.h"
#include "options.h"
#include "descriptions.h"
#include "checksum.h"
#include "images.h"

static size_t pad_to_flash_sector(size_t section_size, unsigned sector_size)
{
  return (section_size / sector_size + 1) * sector_size;
}

static void render_hardware_build_section(struct images *images,
  const struct descriptions *descriptions, unsigned sector_size)
{
  memset(images->hardware_build, 0, IMAGE_MAX);

  struct data_partition_hardware_build build = {
    .tag = DATA_PARTITION_HARDWARE_BUILD_TAG,
    .data = descriptions->hardware_build.build_word,
    .checksum = 0
  };

  build.checksum = checksum_hardware_build_section(&build);

  size_t section_bytes = sizeof(struct data_partition_hardware_build);

  memcpy(images->hardware_build, &build, section_bytes);

  images->hardware_build_size = pad_to_flash_sector(section_bytes,  sector_size);
}

static void render_factory_section(struct images *images,
  const struct descriptions *descriptions, unsigned sector_size,
  bool bad_crc)
{
  memset(images->factory, 0, IMAGE_MAX);

  struct data_partition_image_header header = {
    .tag = DATA_PARTITION_IMAGE_TAG,
    .data_size_words = descriptions->factory.tlv_data_size_words,
    .comp_version = descriptions->factory.comp_version,
    .checksum = 0
  };

  header.checksum = checksum_image(&header, descriptions->factory.tlv_data,
                                   descriptions->factory.tlv_data_size_words);
  if (bad_crc)
    header.checksum = ~header.checksum;

  size_t header_bytes = sizeof(struct data_partition_image_header);
  size_t tlv_bytes = descriptions->factory.tlv_data_size_words * sizeof(int);

  memcpy(images->factory, &header, header_bytes);
  memcpy(images->factory + header_bytes, descriptions->factory.tlv_data, tlv_bytes);

  images->factory_size = pad_to_flash_sector(header_bytes + tlv_bytes, sector_size);
}

static void render_upgrade_section(struct images *images,
  const struct descriptions *descriptions, unsigned sector_size,
  bool bad_crc)
{
  memset(images->upgrade, 0, IMAGE_MAX);

  struct data_partition_image_header header = {
    .tag = DATA_PARTITION_IMAGE_TAG,
    .data_size_words = descriptions->upgrade.tlv_data_size_words,
    .comp_version = descriptions->upgrade.comp_version,
    .checksum = 0
  };

  header.checksum = checksum_image(&header, descriptions->upgrade.tlv_data,
                                   descriptions->upgrade.tlv_data_size_words);
  if (bad_crc)
    header.checksum = ~header.checksum;

  size_t header_bytes = sizeof(struct data_partition_image_header);
  size_t tlv_bytes = descriptions->upgrade.tlv_data_size_words * sizeof(int);

  memcpy(images->upgrade, &header, header_bytes);
  memcpy(images->upgrade + header_bytes, descriptions->upgrade.tlv_data, tlv_bytes);

  images->upgrade_size = pad_to_flash_sector(header_bytes + tlv_bytes, sector_size);
}

void render_descriptions(struct images *images,
                         const struct options *options,
                         const struct descriptions *descriptions)
{
  if (options->factory_file_name != NULL) {
    render_hardware_build_section(images, descriptions,
                                  options->regular_sector_size);

    render_factory_section(images, descriptions,
                           options->regular_sector_size,
                           options->bad_factory_crc);
  }

  if (options->upgrade_file_name != NULL) {
    render_upgrade_section(images, descriptions,
                           options->regular_sector_size,
                           options->bad_upgrade_crc);
  }
}
