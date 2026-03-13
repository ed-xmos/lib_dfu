// Copyright 2020-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef __hal_h__
#define __hal_h__

#include <stddef.h>

#include "control_host.h"
#include "device_id.h"
#if CONTROL_USE_I2C && __xcore__
#include <xccompat.h>
#include "i2c.h"
#endif

#if CONTROL_USE_I2C && __xcore__
int hal_connect(struct device_id device_id, CLIENT_INTERFACE(i2c_master_if, i_i2c));
#else
int hal_connect(struct device_id device_id);
#endif

#if CONTROL_USE_I2C && __xcore__
int hal_read_command(int command, unsigned char payload[], size_t num_bytes, CLIENT_INTERFACE(i2c_master_if, i_i2c));
#else
int hal_read_command(int command, unsigned char payload[], size_t num_bytes);
#endif

#if CONTROL_USE_I2C && __xcore__
int hal_write_command(int command, const unsigned char payload[], size_t num_bytes, CLIENT_INTERFACE(i2c_master_if, i_i2c));
#else
int hal_write_command(int command, const unsigned char payload[], size_t num_bytes);
#endif

#if CONTROL_USE_I2C && __xcore__
int hal_reboot(CLIENT_INTERFACE(i2c_master_if, i_i2c));
#else
int hal_reboot(void);
#endif

#if CONTROL_USE_I2C && __xcore__
int hal_revert_factory(CLIENT_INTERFACE(i2c_master_if, i_i2c));
#else
int hal_revert_factory(void);
#endif

int hal_disconnect(void);

#endif
