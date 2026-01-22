// Copyright 2016-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <platform.h>
#include <stdio.h>
#include <syscall.h>
#include <timer.h>

#include "transport_i2c.h"
#include "i2c.h"
#include "control.h"
#include "app.h"

#ifndef I2C_TILE
#define I2C_TILE 1  // Vision Board: Tile 1, else: tile 0
#endif

on tile[I2C_TILE]: port p_scl = PORT_I2C_SCL;
on tile[I2C_TILE]: port p_sda = PORT_I2C_SDA;

const char i2c_device_addr = 0x2C;

int main(void)
{
  i2c_slave_callback_if i_i2c;
  interface control i_control[1];

  par {
    on tile[I2C_TILE]: par {
      app(i_control[0]);
    }
    on tile[I2C_TILE]: {
      control_init();
      control_register_resources(i_control, 1);

      /* TODO [[combine]] */
      par {
#pragma warning disable unusual-code // Suppress slice interface warning (no array size passed)
        i2c_control_client(i_i2c, i_control);
#pragma warning enable
        i2c_slave(i_i2c, p_scl, p_sda, i2c_device_addr);
      }
    }
  }
  return 0;
}
