// Copyright 2020-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef __sleep_h__
#define __sleep_h__

// Handy macros for prints errors
#define PRINT_ERROR(...)   fprintf(stderr, "Error  : " __VA_ARGS__)
#define PRINT_WARNING(...) fprintf(stderr, "Warning: " __VA_ARGS__)


void sleep_milliseconds(unsigned milliseconds);

#endif
