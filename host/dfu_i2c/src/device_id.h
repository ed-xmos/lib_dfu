// Copyright 2020-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef __device_id_h__
#define __device_id_h__

#include <stdint.h>

struct device_id {
  uint16_t vendor, product, bcddevice;
  uint8_t i2c_address;
};

#endif
