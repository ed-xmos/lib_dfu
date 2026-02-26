// Copyright 2020-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdio.h>
#include <stdbool.h>
#include "control_host.h"
#include "device_id.h"
#include "dfu_utils.h"
#include "labels.h"
#include "hal.h"

#define KWD_BOOT_COMPLETE 1
#define KWD_BOOT_ERROR 2
#define AP_CONTROL_FLAG 1

#define RESOURCE_ID_DFU 0xD0 // Temp - TODO - remove

extern bool quiet;

static int hal_connect_i2c(struct device_id device_id)
{
  const int shift = 0;
  if (control_init_i2c(device_id.i2c_address << shift) != CONTROL_SUCCESS) {
    PRINT_ERROR("Control initialisation over I2C failed\n");
    return 1;
  }
  if (!quiet)
    printf("I2C connected (slave address 0x%X)\n", device_id.i2c_address);

  control_version_t version;
  if (control_read_command(CONTROL_SPECIAL_RESID, CONTROL_GET_VERSION, &version,
                           sizeof(control_version_t)) != CONTROL_SUCCESS) {
    PRINT_ERROR("Control query version failed\n");
    return 2;
  }
  if (version != CONTROL_VERSION) {
    PRINT_ERROR("Mismatch of the control version between host and device.\
                     Expected 0x%X, received 0x%X\n", CONTROL_VERSION, version);
    return 3;
  }
  if (!quiet)
    printf("control version query successful\n");

  return 0;
}

int hal_connect(struct device_id device_id)
{
  return hal_connect_i2c(device_id);
}

int hal_read_command(int command,
                     unsigned char *payload, size_t num_bytes)
{
  if (!quiet) {
    printf("HAL: read command: %s (%d), %lu bytes\n",
           command_str(command), command, num_bytes);
  }

  if (control_read_command(RESOURCE_ID_DFU, CONTROL_CMD_SET_READ(command),
                           payload, num_bytes) != CONTROL_SUCCESS) {
    PRINT_ERROR("Control read command did not return success\n");
    return 1;
  }

  return 0;
}

int hal_write_command(int command,
                      const unsigned char *payload, size_t num_bytes)
{
  if (!quiet) {
    printf("HAL: write command: %s (%d), %lu bytes\n",
           command_str(command), command, num_bytes);
  }

  // support empty payload at the HAL level without relying on underlying code
  unsigned char null[1];
  const unsigned char *payload_or_null = payload == NULL ? null : payload;

  if (control_write_command(RESOURCE_ID_DFU, CONTROL_CMD_SET_WRITE(command),
                            payload_or_null, num_bytes) != CONTROL_SUCCESS) {
    PRINT_ERROR("Control write command did not return success\n");
    return 1;
  }

  return 0;
}

int hal_reboot(void)
{
  if (!quiet)
    printf("HAL: reboot\n");

  if (hal_write_command(DFU_CMD_BUS_RESET, NULL, 0) != 0)
    return 1;

  return 0;
}

int hal_disconnect(void)
{
  if (control_cleanup_i2c() != CONTROL_SUCCESS)
    return 1;

  return 0;
}
