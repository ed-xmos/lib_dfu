// Copyright 2020-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "dfu_utils.h"
#if defined(_MSC_VER)
#include <Winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#endif

void sleep_milliseconds(unsigned milliseconds)
{
#if defined(_MSC_VER)
  Sleep(milliseconds);
#else
  usleep(milliseconds * 1000);
#endif
}
