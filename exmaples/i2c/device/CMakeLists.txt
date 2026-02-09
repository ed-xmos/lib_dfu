cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(i2c)

# target: choose the target platform
set(APP_HW_TARGET XK-EVK-XU316-AIV_i2c.xn)

# Includes and flags
set(APP_INCLUDES src ../shared)

# To enable I2C adapter in the lib_device_control library, this application must build 'i2c' configuration
set(TRANSPORT_CONFIG  "I2C")
set(CONTROL_APP_DFU   ON)

set(APP_COMPILER_FLAGS
  -lquadflash  # TODO - move this to the lib_dfu module
  -O2
  -g
  -report
  -Wall
  -Wextra
)

set(APP_DEPENDENT_MODULES   lib_dfu "lib_i2c(6.4.1)" "lib_device_control(develop)")

set(XMOS_SANDBOX_DIR        ${CMAKE_CURRENT_LIST_DIR}/../../../..)

XMOS_REGISTER_APP()
