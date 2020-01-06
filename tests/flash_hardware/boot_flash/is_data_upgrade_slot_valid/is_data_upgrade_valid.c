// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <platform.h>
#include <print.h>
#include <stdbool.h>
#include <stdio.h>
#include <xmos_flash.h>
#include "sqi_access.h"

#define XASSERT_ENABLE_DEBUG 1
#define XASSERT_ENABLE_LINE_NUMBERS 1
#include "xassert.h"

#define DEBUG_UNIT TEST
#define DEBUG_PRINT_ENABLE_TEST 0
#include "debug_print.h"

#include "boot_flash.h"

flash_clock_config_t clock_config = {
  flash_clock_xcore,
  3, // XCore/3/3/falling/0 for 12.5MHz (see XFlash_Options.cpp)
  3,
  flash_clock_input_edge_falling,
  flash_port_pad_delay_0
};

flash_handle_t flash_handle;

flash_ports_t flash_ports = {
  PORT_SQI_CS,
  PORT_SQI_SCLK,
  PORT_SQI_SIO,
  XS1_CLKBLK_1
};

flash_qe_config_t qe_config = { // 0/6 for XCORE-200-EXPLORER (see XN file)
  flash_qe_location_status_reg_0,
  flash_qe_bit_6
};

unsigned DEVICE_SECTOR_WORD_SIZE = 1024; // IS25LQ016B

int main(int argc, char **argv)
{
  int ret;
  bool valid = false;
  bool expected;

  assert(argc == 2);

  DeviceAccess_Connect();

  ret = flash_is_data_upgrade_slot_valid(&valid);
  assert(ret == 0);

  DeviceAccess_Disconnect(0);

  expected = (argv[1][0] == 'V');

  if (expected != valid) {
    debug_printf("expected %d actual %d\n", expected, valid);
    assert(0);
  }

  printstr("PASS\n");
  return 0;
}
