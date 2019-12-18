// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include "data_partition.h"
#include "crc.h"
#include "checksum.h"

void checksum_hardware_build_section(struct data_partition_hardware_build *build)
{
  size_t section_words = sizeof(struct data_partition_hardware_build) / sizeof(int);
  unsigned crc;

  build->checksum = 0;
  crc = crc_init();
  for (int i = 0; i < section_words; i++) {
    crc = crc_step(crc, ((unsigned*)build)[i]);
  }

  build->checksum = crc_finish(crc);

  crc = crc_init();
  for (int i = 0; i < section_words; i++) {
    crc = crc_step(crc, ((unsigned*)build)[i]);
  }
  assert(crc_finish(crc) == 0);
}

void checksum_image(struct data_partition_image_header *header,
                    const uint8_t *data, size_t data_num_words)
{
  size_t header_words =
    sizeof(struct data_partition_image_header) / sizeof(int);

  int header_checksum_word_offset =
    offsetof(struct data_partition_image_header, checksum) / sizeof(int);

  assert(sizeof(struct data_partition_image_header) % sizeof(int) == 0);
  assert(offsetof(struct data_partition_image_header, checksum) % sizeof(int) == 0);

  unsigned crc;

  crc = crc_init();
  for (int i = 0; i < header_words; i++) {
    if (i != header_checksum_word_offset)
      crc = crc_step(crc, ((unsigned*)header)[i]);
  }
  for (int i = 0; i < data_num_words; i++) {
    crc = crc_step(crc, ((unsigned*)data)[i]);
  }
  crc = crc_step(crc, 0); // checksum moved after data

  header->checksum = crc_finish(crc);

  crc = crc_init();
  for (int i = 0; i < header_words; i++) {
    if (i != header_checksum_word_offset)
      crc = crc_step(crc, ((unsigned*)header)[i]);
  }
  for (int i = 0; i < data_num_words; i++) {
    crc = crc_step(crc, ((unsigned*)data)[i]);
  }
  crc = crc_step(crc, header->checksum);
  assert(crc_finish(crc) == 0);
}
