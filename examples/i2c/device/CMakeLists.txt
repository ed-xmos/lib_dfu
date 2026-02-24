cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(i2c)

# target: choose the target platform
set(APP_HW_TARGET           xk-voice-l71.xn)

# Includes and flags
set(APP_INCLUDES            src ../shared)

# To enable I2C adapter in the lib_device_control library, this application must build 'i2c' configuration
set(TRANSPORT_CONFIG        "I2C")
set(CONTROL_APP_DFU         ON)

set(APP_COMPILER_FLAGS      -O2
                            -g
                            -lquadflash
                            -report
                            -Wall
                            -Wextra
)

include(../deps.cmake)

set(XMOS_SANDBOX_DIR        ${CMAKE_CURRENT_LIST_DIR}/../../../..)

XMOS_REGISTER_APP()
