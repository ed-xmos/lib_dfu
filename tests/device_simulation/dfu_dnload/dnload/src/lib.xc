// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <xs1.h>

void crc32_c(unsigned &checksum, unsigned data, unsigned poly);

void crc32_c(unsigned &checksum, unsigned data, unsigned poly) {
    crc32(checksum, data, poly);
}
