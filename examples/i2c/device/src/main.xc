// Copyright 2016-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <platform.h>
#include <stdio.h>
#include <syscall.h>
#include <timer.h>

#include "transport_i2c.h"
#include "i2c.h"
#include "control.h"
#include "app.h"
#include "resource.h"
#include "dfu_control_server.h"

on tile[PORT_I2C_SCL_TILE_NUM]: port p_scl = PORT_I2C_SCL;
on tile[PORT_I2C_SDA_TILE_NUM]: port p_sda = PORT_I2C_SDA;

int main(void)
{
  i2c_slave_callback_if i_i2c;
  interface control i_control[2];

  par {
    on tile[0]: par {
      dfu_control_server(i_control[1]);
    }
    on tile[1]: par {
      app(i_control[0]);
    }
    on tile[PORT_I2C_SCL_TILE_NUM]: {
      control_init();
      control_register_resources(i_control, 2);

      /* bug 17317 - [[combine]] */
      par {
#pragma warning disable unusual-code // Suppress slice interface warning (no array size passed)
        i2c_control_client(i_i2c, i_control);
#pragma warning enable
        i2c_slave(i_i2c, p_scl, p_sda, DEVICE_I2C_ADDRESS);
      }
    }
  }
  return 0;
}

/* TODO list 
 * 
 * Timings to fix:
 * write of first 128 bytes, pauses/clock stretch of 1ms - could this be flash_connect()? profile (105537) -> 1.05ms
 * get status following first packet, pauses 111ms - first erase?
 * - could this be fl_startImageReplace()? profile (11138915) -> 111.3ms
 * - or fl_startImageAdd()? profile.
 * get status on transition to DOWNLOAD-IDLE, pauses 38ms - could this be fl_writeImagePage()? profile. Why is first write is slow? (3751599) -> 37.5ms
 * get status after each page downloaded, pauses 0.5ms - could this be fl_writeImagePage()? profile.
 * 
 * bus-reset also does not return so holds the clock for 50ms.
 * 
 */
