// Copyright 2020-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "hal.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "control_host.h"
#include "device_id.h"
#include "dfu_utils.h"
#include "labels.h"

#define KWD_BOOT_COMPLETE 1
#define KWD_BOOT_ERROR 2
#define AP_CONTROL_FLAG 1

extern int quiet;

static uint8_t buffer[256];

#if USE_I2C && __xcore__
int hal_connect(struct device_id device_id, CLIENT_INTERFACE(i2c_master_if, i_i2c))
#else
int hal_connect(struct device_id device_id)
#endif
{
  const int shift = 0;
  if (control_init_i2c(device_id.i2c_address << shift) != CONTROL_SUCCESS) {
    PRINT_ERROR("Control initialisation over I2C failed\n");
    return 1;
  }
  if (!quiet) {
    printf("I2C connected (slave address 0x%X)\n", device_id.i2c_address);
  }

  control_version_t version;
#if USE_I2C && __xcore__
  if (control_read_command(CONTROL_SPECIAL_RESID, CONTROL_GET_VERSION, i_i2c, &version, sizeof(control_version_t)) != CONTROL_SUCCESS)
#else
  if (control_read_command(CONTROL_SPECIAL_RESID, CONTROL_GET_VERSION, &version, sizeof(control_version_t)) != CONTROL_SUCCESS)
#endif
  {
    PRINT_ERROR("Control query version failed\n");
    return 2;
  }
  if (version != CONTROL_VERSION) {
    PRINT_ERROR("Mismatch of the control version between host and device. Expected 0x%X, received 0x%X\n", CONTROL_VERSION, version);
    return 3;
  }
  if (!quiet) {
    printf("control version query successful\n");
  }

  return 0;
}

#if USE_I2C && __xcore__
int hal_read_command(int command, unsigned char payload[], size_t num_bytes, CLIENT_INTERFACE(i2c_master_if, i_i2c))
#else
int hal_read_command(int command, unsigned char payload[], size_t num_bytes)
#endif
{
  if (!quiet) {
    printf("HAL: read command: %s (%d), %zu bytes\n", command_str(command), command, num_bytes);
  }
  if (num_bytes == 0 || payload == NULL) {
    PRINT_ERROR("Payload pointer is NULL for non-zero payload length\n");
    return 1;
  }

#if USE_I2C && __xcore__
  if (control_read_command(RESOURCE_ID_DFU, CONTROL_CMD_SET_READ(command), i_i2c, buffer, (num_bytes + sizeof(struct dfu_upload_header))) != CONTROL_SUCCESS)
#else
  if (control_read_command(RESOURCE_ID_DFU, CONTROL_CMD_SET_READ(command), buffer, (num_bytes + sizeof(struct dfu_upload_header))) != CONTROL_SUCCESS)
#endif
  {
    PRINT_ERROR("Control read command did not return success\n");
    return 1;
  }
  struct dfu_upload_header header;
  memcpy(&header, buffer, sizeof(header));
  if (header.read_length != num_bytes) {
    PRINT_ERROR("Received %u bytes, expected %zu bytes\n", header.read_length, num_bytes);
    return 1;
  } else {
    printf("received: length %u, pad 0x%04X\n", header.read_length, header.pad);
    memcpy(payload, buffer + sizeof(header), num_bytes);
  }

  return 0;
}

#if USE_I2C && __xcore__
int hal_write_command(int command, const unsigned char payload[], size_t num_bytes, CLIENT_INTERFACE(i2c_master_if, i_i2c))
#else
int hal_write_command(int command, const unsigned char payload[], size_t num_bytes)
#endif
{
  /* TODO - check this, block_num increments for every write */
  static uint16_t block_num = 0;

  if (!quiet) {
    printf("HAL: write command: %s (%d), %zu bytes\n", command_str(command), command, num_bytes);
  }
  if (num_bytes != 0 && payload == NULL) {
    PRINT_ERROR("Payload pointer is NULL for non-zero payload length\n");
    return 1;
  }
  size_t payload_bytes = 0;

  if (num_bytes > ((sizeof(buffer) - sizeof(struct dfu_dnload_header)))) {
    PRINT_ERROR("Payload size %zu is too large. Maximum supported is %zu bytes\n", num_bytes, (sizeof(buffer) - sizeof(struct dfu_dnload_header)));
    return 1;

  } else if (num_bytes != 0)  {
    struct dfu_dnload_header header = { 0, 0 };
    header.block_num = ++block_num;

    payload_bytes = num_bytes + sizeof(header);
    memcpy(buffer, &header, sizeof(header));
    if (num_bytes > 0) {
      memcpy(buffer + sizeof(header), payload, num_bytes);
    }
  }

#if USE_I2C && __xcore__
  if (control_write_command(RESOURCE_ID_DFU, CONTROL_CMD_SET_WRITE(command), i_i2c, buffer, payload_bytes) != CONTROL_SUCCESS)
#else
  if (control_write_command(RESOURCE_ID_DFU, CONTROL_CMD_SET_WRITE(command), buffer, payload_bytes) != CONTROL_SUCCESS)
#endif
  {
    PRINT_ERROR("Control write command did not return success\n");
    return 1;
  }

  return 0;
}

#if USE_I2C && __xcore__
int hal_reboot(CLIENT_INTERFACE(i2c_master_if, i_i2c))
{
  if (!quiet) {
    printf("HAL: reboot\n");
  }

  if (hal_write_command(XMOS_BUS_RESET, NULL, 0, i_i2c) != 0) {
    return 1;
  }

  return 0;
}
#else
int hal_reboot(void)
{
  if (!quiet) {
    printf("HAL: reboot\n");
  }

  if (hal_write_command(XMOS_BUS_RESET, NULL, 0) != 0) {
    return 1;
  }

  return 0;
}
#endif

#if USE_I2C && __xcore__
int hal_revert_factory(CLIENT_INTERFACE(i2c_master_if, i_i2c))
#else
int hal_revert_factory(void)
#endif
{
  if (!quiet) {
    printf("HAL: revert factory\n");
  }

  if (hal_write_command(XMOS_DFU_REVERTFACTORY, NULL, 0) != 0) {
    return 1;
  }
  return 0;
}

int hal_disconnect(void)
{
  if (control_cleanup_i2c() != CONTROL_SUCCESS) {
    return 1;
  }

  return 0;
}
