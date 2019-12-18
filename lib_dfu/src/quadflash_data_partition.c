// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <quadflash.h>
#include <quadflashlib.h>

#define DEBUG_UNIT QUADFLASH_DATA_PARTITION
#define DEBUG_PRINT_ENABLE_QUADFLASH_DATA_PARTITION 1
#include "debug_print.h"

#include "data_partition.h"
#include "quadflash_internal.h"
#include "quadflash_data_partition.h"

static const fl_QuadDeviceSpec* g_flashAccess = NULL;

void fl_saveSpecPointer(const fl_QuadDeviceSpec spec[1])
{
  g_flashAccess = spec;
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

  if (header.tag != DATA_PARTITION_IMAGE_TAG)
    return 1;

  // TODO CRC check

  dataImageInfo->startAddress = header_address;
  dataImageInfo->factory = 1;
  dataImageInfo->version = header.comp_version;

  // return image size that's like the one returned by getFactoryImage
  // it includes header and data but excludes sector alignment padding
  dataImageInfo->size = sizeof(struct data_partition_image_header) +
    header.data_size_words * sizeof(uint32_t);

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

    // stop as soon as we find a subsequent sector that is not an image header
    if (header.tag != DATA_PARTITION_IMAGE_TAG)
      return 1;

    // TODO CRC check
    
    if (1) {
      dataImageInfo->startAddress = header_address;
      dataImageInfo->factory = 0;
      dataImageInfo->version = header.comp_version;
      return 0;
    }

    // skip over this image based on length given
    sector = fl_getSectorAtOrAfter(header_address +
      sizeof(struct data_partition_image_header) +
      header.data_size_words * sizeof(uint32_t));
  }

  return 1;
}
