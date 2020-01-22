// Copyright (c) 2020, XMOS Ltd, All rights reserved
#ifndef __dfu_suffix_h__
#define __dfu_suffix_h__

#include <stdint.h>

#define DFU_SIGNATURE {0x44, 0x46, 0x55}
#define DFU_BCD 0x0110

#pragma pack(push, 1)

struct dfu_suffix {
  // Checksum of file excluding suffix (specification does include the suffix).
  uint32_t crc;

  // The length of this DFU suffix including dwCRC.
  uint8_t suffix_length;

  // The unique DFU signature field.
  uint8_t signature[3];

  // DFU specification number.
  uint16_t bcd_dfu;

  // The vendor ID associated with this file. Either FFFFh or must match
  // device's vendor ID.
  uint16_t vendor_id;

  // The product ID associated with this file. Either FFFFh or must match
  // device's product ID.
  uint16_t product_id;

  // The release number of the device associated with this file. Either FFFFh
  // or a BCD firmware release or version number.
  uint16_t bcd_device;
};

#pragma pack(pop)

#endif
