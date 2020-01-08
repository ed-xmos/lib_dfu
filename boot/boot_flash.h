// Copyright (c) 2020, XMOS Ltd, All rights reserved
//
// For documentation behind xflash see libquadflash and old TWiki pages.
// See also libstage2loader.
//
// Notes:
//
// - It is only necessary to implement single page CRC validation. Stage 2
// loader (Stage2Loader_ValidateImage.c) would already have validated entire
// image CRC at the point checkCandidateImageVersion or recordCandidateImage
// are called.
//
// - DFU library has a function similar to 'find upgrade image'. One difference
// is that the DFU function does not require a valid upgrade slot. It skips
// over factory image, and returns sector-aligned address of where upgrade
// image would go if there was one. This is appropriate for writing as part of
// firmware upgrade. For the purposes of booting, however we require the
// upgrade image be valid, because we are about to boot from it.
//
// - SQI functions nibble swap, but xflash convention is no nibble swap in data
// partition. Therefore do an extra nibble swap here.
//
#ifndef __boot_flash_h__
#define __boot_flash_h__
#ifdef BOOT_FLASH_UNIT_TEST

int flash_find_factory_image(unsigned *address);
int flash_find_upgrade_image(unsigned *address, unsigned factory_start);
int flash_is_data_upgrade_slot_valid(bool *valid);

#else

#include <stdint.h>
#include <stddef.h>
#include <print.h>

#ifdef __XC__
#define _Bool int
#endif
#include <stdbool.h>

#include "sqi_access.h"
#include "data_partition_structures.h"
#include "boot_flash_util.h"

#define CRC_PAGE_SIZE_WORDS 64
#define XFLASH_IMAGE_TAG 0x0FF51DE
#define ROM_QSPI_SIZE_WORD 0
#define ROM_QSPI_EXTRA_WORDS 2 // size word and CRC word

enum xflash_image_header_word_offsets {
  TAG = 0,
  PAGE_CRC = 1,
  CRC_START = 3,
  IMAGE_SIZE = 4
};

// closely based on fl_getFactoryImage (specialised to tools 14-style header)
static inline int flash_find_factory_image(unsigned *address)
{
  unsigned header = DeviceAccess_ReadWord(ROM_QSPI_SIZE_WORD) + ROM_QSPI_EXTRA_WORDS;

  unsigned tag = DeviceAccess_ReadWord(header + TAG);
  if (tag != XFLASH_IMAGE_TAG)
    return 1;

  unsigned page_crc = DeviceAccess_ReadWord(header + PAGE_CRC);
  unsigned result = DeviceAccess_Streamed_CRC(header + CRC_START,
                                              CRC_PAGE_SIZE_WORDS - CRC_START,
                                              page_crc);
  if (result != 0)
    return 2;

  *address = header * sizeof(uint32_t);

  unsigned image_size = DeviceAccess_ReadWord(header + IMAGE_SIZE);
  if (*address + image_size > get_boot_partition_size())
    return 3;

  return 0;
}

// loosely based on fl_getNextBootImage (specialised to tools 14-style header)
static inline int flash_find_upgrade_image(unsigned *address,
                                           unsigned factory_start)
{
  unsigned factory_size = DeviceAccess_ReadWord((factory_start >> 2) + IMAGE_SIZE);
  unsigned header = sector_address_at_or_after(factory_start + factory_size) /
                    sizeof(uint32_t);

  unsigned tag = DeviceAccess_ReadWord(header + TAG);
  if (tag != XFLASH_IMAGE_TAG)
    return 1;

  unsigned page_crc = DeviceAccess_ReadWord(header + PAGE_CRC);
  unsigned result = DeviceAccess_Streamed_CRC(header + CRC_START,
                                              CRC_PAGE_SIZE_WORDS - CRC_START,
                                              page_crc);
  if (result != 0)
    return 2;

  *address = header * sizeof(uint32_t);
  return 0;
}

static inline int checksum_data_image(unsigned header_address,
                                      unsigned checksum_offset,
                                      unsigned size_bytes)
{
  unsigned addr = header_address;
  unsigned read = 0;
  unsigned checksum = 0;
  unsigned crc = crc_init();
  unsigned buf[CRC_PAGE_SIZE_WORDS];

  if (checksum_offset >= size_bytes || checksum_offset % sizeof(int) != 0)
    return 1;

  while (read < size_bytes) {
    DeviceAccess_Read(addr >> 2, buf, sizeof(buf) >> 2);

    unsigned valid = sizeof(buf);
    if (read + sizeof(buf) > size_bytes)
      valid = size_bytes - read;

    for (int i = 0; i < valid / sizeof(int); i++) {
      if (read + i * sizeof(int) == checksum_offset)
        checksum = nibble_swap_word(buf[i]);
      else
        crc_step(&crc, nibble_swap_word(buf[i]));
    }
    read += valid;
  }

  crc_step(&crc, checksum);

  if (crc_finish(crc) != 0)
    return 2;

  return 0;
}

static int validate_data_image(const struct data_partition_image_header *header,
                               unsigned header_address, unsigned *size)
{
  if (nibble_swap_word(header->tag) != DATA_PARTITION_IMAGE_TAG)
    return 1;

  *size = sizeof(struct data_partition_image_header) + // excludes sector padding
    nibble_swap_word(header->data_size_words) * sizeof(uint32_t);

  unsigned checksum_offset = offsetof(struct data_partition_image_header, checksum);
  if (checksum_data_image(header_address, checksum_offset, *size) != 0)
    return 2;

  return 0;
}

// loosely based on fl_getNextBootImage and
// the derived fl_getNextDataImage in DFU library
static inline int flash_is_data_upgrade_slot_valid(bool *valid)
{
  struct data_partition_image_header header;
  int ret;

  unsigned end_of_hardware_build = get_boot_partition_size() +
                                   sizeof(struct data_partition_hardware_build);

  unsigned factory_start = sector_address_at_or_after(end_of_hardware_build);

  DeviceAccess_Read(factory_start >> 2, (void*)&header,
                    sizeof(struct data_partition_image_header) >> 2);

  unsigned factory_size = 0;
  ret = validate_data_image(&header, factory_start, &factory_size);
  if (ret != 0)
    return 1;

  unsigned upgrade_address = sector_address_at_or_after(factory_start + factory_size);

  DeviceAccess_Read(upgrade_address >> 2, (void*)&header,
                    sizeof(struct data_partition_image_header) >> 2);

  unsigned upgrade_size = 0;
  ret = validate_data_image(&header, upgrade_address, &upgrade_size);

  *valid = (ret == 0);
  return 0;
}

#endif
#endif
