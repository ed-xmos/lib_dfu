// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "dfu_flash.h"

#include "dfu.h"

enum flash_status flash_cmd_enable_ports() __attribute__((weak));
enum flash_status flash_cmd_enable_ports() { return DFU_FLASH_OPEN_ERROR; }

enum flash_status flash_cmd_disable_ports() __attribute__((weak));
enum flash_status flash_cmd_disable_ports() { return DFU_FLASH_OPEN_ERROR; }

void DFUCustomFlashEnable() __attribute__((weak));
void DFUCustomFlashEnable() {}

void DFUCustomFlashDisable() __attribute__((weak));
void DFUCustomFlashDisable() {}
