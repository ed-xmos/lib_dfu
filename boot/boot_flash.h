// Copyright (c) 2020, XMOS Ltd, All rights reserved
#ifndef __boot_flash_h__
#define __boot_flash_h__

#include <stdint.h>
#include <print.h>

#ifdef __XC__
#define _Bool int
#endif
#include <stdbool.h>

#include "sqi_access.h"
#include "data_partition_structures.h"

// see quadflashlib.c or the xflash ImageHeaderFormat wiki page
enum xflash_image_header_word_offsets {
  TAG = 0,
  PAGE_CRC = 1,
  CRC_START = 3,
  IMAGE_SIZE = 4
};

// copied from libstage2loader (Stage2Loader_CRC.c)
#define CRC_PAGE_SIZE_WORDS 64

// copied from SQI library (DeviceAccess_SQI.xc)
static unsigned char nibble_swap_byte(unsigned char c)
{
  return ((c & 0x0F) << 4) | ((c & 0xF0) >> 4);
}

static unsigned int nibble_swap_word(unsigned int x)
{
  unsigned int y = 0;
  y = (nibble_swap_byte((x & 0xFF000000) >> 24) << 24);
  y |= (nibble_swap_byte((x & 0x00FF0000) >> 16) << 16);
  y |= (nibble_swap_byte((x & 0x0000FF00) >> 8) << 8);
  y |= nibble_swap_byte(x & 0x000000FF);
  return y;
}

// closely based on fl_getFactoryImage
// specialised to tools 14-style header
static inline int flash_find_factory_image(unsigned *address)
{
  // word zero is word size
  // what follows is that many words of stage 2 loader and a CRC
  // factory image follows immediately, so add 2 words - word zero and CRC word
  unsigned header = DeviceAccess_ReadWord(0) + 2;

  unsigned tag = DeviceAccess_ReadWord(header + TAG);
  if (tag != 0x0FF51DE)
    return 1;

  // only validate page CRC like fl_getFactoryImage
  // Stage2Loader_ValidateImage validates image CRC
  // and that has already happened at this point in xflash custom loader
  unsigned page_crc = DeviceAccess_ReadWord(header + PAGE_CRC);
  unsigned result = DeviceAccess_Streamed_CRC(header + CRC_START,
                                              CRC_PAGE_SIZE_WORDS - CRC_START, 
                                              page_crc);
  if (result != 0)
    return 2;

  *address = header << 2;
  return 0;
}

static inline unsigned sector_address_at_or_after(unsigned address)
{
  unsigned sector_size = DEVICE_SECTOR_WORD_SIZE * 4;
  if (address % sector_size == 0)
    return address; // at
  else
    return (address / sector_size + 1) * sector_size; // or after
}

// loosely based on fl_getNextBootImage
// specialised to tools 14-style header
// some checks omitted such as whether search stays within boot partition
// requires a valid upgrade slot unlike similar function in DFU library
// another difference is that it doesn't search for factory image again
static inline int flash_find_upgrade_image(unsigned *address,
                                           unsigned factory_start)
{
  unsigned factory_size = DeviceAccess_ReadWord((factory_start >> 2) + IMAGE_SIZE);
  unsigned header = sector_address_at_or_after(factory_start + factory_size) >> 2;

  unsigned tag = DeviceAccess_ReadWord(header + TAG);
  if (tag != 0x0FF51DE)
    return 1;

  unsigned page_crc = DeviceAccess_ReadWord(header + PAGE_CRC);
  unsigned result = DeviceAccess_Streamed_CRC(header + CRC_START,
                                              CRC_PAGE_SIZE_WORDS - CRC_START, 
                                              page_crc);
  if (result != 0)
    return 2;

  *address = header << 2;
  return 0;
}

static int validateDataImage(const struct data_partition_image_header *header,
                             unsigned header_address, unsigned *size)
{
  if (nibble_swap_word(header->tag) != DATA_PARTITION_IMAGE_TAG)
    return 1;

  // image size like the one returned by fl_getFactoryImage
  // includes header and data but excludes sector alignment padding
  *size = sizeof(struct data_partition_image_header) +
    nibble_swap_word(header->data_size_words) * sizeof(uint32_t);

  // TODO checksum

  return 0;
}

// loosely based on fl_getNextBootImage, as well as the
// derived fl_getNextDataImage in DFU library
// note: SQI functions nibble swap, but xflash convention is no nibble swap
// in data partition; therefore do an extra nibble swap here
static inline int flash_is_data_upgrade_slot_valid(bool *valid)
{
  struct data_partition_image_header header;
  int ret;

  // skip over sector-aligned hardware build section
  unsigned factory_address = sector_address_at_or_after(1048576 + // TODO data partition base
      sizeof(struct data_partition_hardware_build));

  DeviceAccess_Read(factory_address >> 2, (void*)&header,
                    sizeof(struct data_partition_image_header) >> 2);

  unsigned factory_size = 0;
  ret = validateDataImage(&header, factory_address, &factory_size);
  if (ret != 0)
    return 1;

  unsigned upgrade_address = sector_address_at_or_after(factory_address + factory_size);

  DeviceAccess_Read(upgrade_address >> 2, (void*)&header,
                    sizeof(struct data_partition_image_header) >> 2);

  unsigned upgrade_size = 0;
  ret = validateDataImage(&header, upgrade_address, &upgrade_size);

  *valid = (ret == 0);
  return 0;
}

#endif
