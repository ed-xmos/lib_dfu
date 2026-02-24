// Copyright 2016-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <platform.h>
#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include <assert.h>
#include <timer.h>

#include "control.h"
#include "dfu.h"
#include "i2c.h"
#include "control_host.h"
#include "resource.h"
#include "control_host_util.h"

port p_scl = on tile[0]: XS1_PORT_1N; // Can be accessed via signal SCL_3V3, TP13
port p_sda = on tile[0]: XS1_PORT_1O; // Can be accessed via signal SDA_3V3, TP14

#define GET_STATE_LENGTH_BYTES 5

int dfu_getState(client i2c_master_if i_i2c, unsigned char *state)
{
  unsigned char payload[GET_STATE_LENGTH_BYTES];
  int ctrl = control_read_command(RESOURCE_ID_DFU, CONTROL_CMD_SET_READ(DFU_GETSTATE), i_i2c, payload, GET_STATE_LENGTH_BYTES);
  if (ctrl != CONTROL_SUCCESS) {
    printf("control read state command failed with %d\n", ctrl);
    exit(1);
  }
  *state = payload[4];
  return 0;
}

// int dfu_getStatus(unsigned int interface, unsigned char *state, unsigned int *timeout,
//                   unsigned char *nextState, unsigned char *strIndex)
// {
//     unsigned int data[2];
//     libusb_control_transfer(devh, USB_BMREQ_D2H_CLASS_INT, DFU_GETSTATUS, 0, interface, (unsigned char *)data, 6, 0);

//     *state = data[0] & 0xff;
//     *timeout = (data[0] >> 8) & 0xffffff;
//     *nextState = data[1] & 0xff;
//     *strIndex = (data[1] >> 8) & 0xff;
//     return 0;
// }

int main(void)
{
  i2c_master_if i_i2c[1];
  par {
    on tile[0]: {
      i2c_master(i_i2c, 1, p_scl, p_sda, 100);
    }
    on tile[1]: {
      control_version_t version;
      unsigned char payload[4];
      int i;

      if (control_init_i2c(DEVICE_I2C_ADDRESS) != CONTROL_SUCCESS) {
        printf("control init failed\n");
        exit(1);
      }

      printf("i2c ready\n");

      if (control_query_version(&version, i_i2c[0]) != CONTROL_SUCCESS) {
        printf("control query version failed\n");
        exit(1);
      }
      if (version != CONTROL_VERSION) {
        printf("version expected 0x%X, received 0x%X\n", CONTROL_VERSION, version);
      }

      printf("started\n");

      dfu_getState(i_i2c[0], &payload[0]);
      printf("DFU state: %d\n", payload[0]);

      for (i = 0; i < 4; i++) {
        payload[0] = i;
        if (control_write_command(RESOURCE_ID, CONTROL_CMD_SET_WRITE(0), i_i2c[0], payload, 1) != CONTROL_SUCCESS) {
          printf("control write command failed\n");
          exit(1);
        }

        pause_short();

        if (control_read_command(RESOURCE_ID, CONTROL_CMD_SET_READ(0), i_i2c[0], payload, 1) != CONTROL_SUCCESS) {
          printf("control read command failed\n");
          exit(1);
        }

        pause_long();

        if (payload[0] != i) {
          printf("control read command returned the wrong value, expected %d, returned %d\n", i, payload[0]);
          exit(1);
        }
        printf("Written and read back command with payload: 0x%02X\n", payload[0]);
      }

      control_cleanup_i2c();
      printf("done\n");
    }
  }
  return 0;
}