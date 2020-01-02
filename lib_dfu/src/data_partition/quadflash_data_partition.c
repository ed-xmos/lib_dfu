// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
#ifndef QUADFLASH_DATA_PARTITION_UNIT_TEST
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <quadflash.h>
#include <quadflashlib.h>

#define DEBUG_UNIT QUADFLASH_DATA_PARTITION
#define DEBUG_PRINT_ENABLE_QUADFLASH_DATA_PARTITION 0
#include "debug_print.h"

#include "data_partition.h"
#include "quadflash_crc.h"
#include "quadflash_internal.h"
#include "quadflash_data_partition.h"

static const fl_QuadDeviceSpec* g_flashAccess = NULL;

void fl_saveSpecPointer(const fl_QuadDeviceSpec spec[1])
{
  g_flashAccess = spec;
}

int fl_dataImageChecksum(unsigned header_address, unsigned checksum_offset,
                         unsigned size_bytes)
{
  unsigned addr = header_address;
  unsigned read = 0;
  unsigned checksum = 0;
  unsigned crc = crc_init();
  unsigned buf[16];

  if (checksum_offset >= size_bytes || checksum_offset % sizeof(int) != 0)
    return 1;

  while (read < size_bytes) {
    fl_int_read(g_flashAccess->readCommand, addr, (unsigned char*)buf, sizeof(buf));

    unsigned valid = sizeof(buf);
    if (read + sizeof(buf) > size_bytes)
      valid = size_bytes - read;

    for (int i = 0; i < valid / sizeof(int); i++) {
      if (read + i * sizeof(int) == checksum_offset)
        checksum = buf[i];
      else
        crc_step(&crc, buf[i]);
    }
    read += valid;
  }

  crc_step(&crc, checksum);

  if (crc_finish(crc) != 0)
    return 1;

  return 0;
}

static int validateDataImage(const struct data_partition_image_header *header,
                             unsigned header_address, unsigned *size)
{
  if (header->tag != DATA_PARTITION_IMAGE_TAG)
    return 1;

  // image size like the one returned by fl_getFactoryImage
  // includes header and data but excludes sector alignment padding
  *size = sizeof(struct data_partition_image_header) +
    header->data_size_words * sizeof(uint32_t);

  unsigned checksum_offset = offsetof(struct data_partition_image_header, checksum);
  if (fl_dataImageChecksum(header_address, checksum_offset, *size) != 0)
    return 2;

  return 0;
}

// closely based on fl_getFactoryImage
int fl_getFactoryDataImage(fl_DataImageInfo *dataImageInfo)
{
  struct data_partition_image_header header;

  // skip over sector-aligned hardware build section
  unsigned header_address =
    fl_getSectorAddress(fl_getSectorContaining(fl_getDataPartitionBase() +
      sizeof(struct data_partition_hardware_build)) + 1);

  fl_int_read(g_flashAccess->readCommand, header_address,
              (void*)&header, sizeof(struct data_partition_image_header));

  unsigned size;
  int ret = validateDataImage(&header, header_address, &size);
  if (ret != 0)
    return 1;

  dataImageInfo->startAddress = header_address;
  dataImageInfo->factory = 1;
  dataImageInfo->version = header.comp_version;
  dataImageInfo->size = size;

  return 0;
}

// loosely based on fl_getNextBootImage
int fl_getNextDataImage(fl_DataImageInfo *dataImageInfo)
{
  struct data_partition_image_header header;
  unsigned sector = fl_getSectorAtOrAfter(dataImageInfo->startAddress +
                                          dataImageInfo->size);
  while (sector < fl_getNumSectors()) {
    unsigned header_address = fl_getSectorAddress(sector);
    fl_int_read(g_flashAccess->readCommand, header_address,
                (void*)&header, sizeof(struct data_partition_image_header));

    unsigned size = 0;
    int ret = validateDataImage(&header, header_address, &size);

    if (ret == 0) {
      dataImageInfo->startAddress = header_address;
      dataImageInfo->factory = 0;
      dataImageInfo->version = header.comp_version;
      dataImageInfo->size = size;
      return 0;
    }

    // probably subsequent sector that is not an image header
    // don't know size so cannot skip over image - end here
    if (ret != 2 || size == 0)
      break;

    // probably checksum fail
    // know size so can skip over image
    sector = fl_getSectorAtOrAfter(header_address + size);
  }

  return 1;
}
#endif
