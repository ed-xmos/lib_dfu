cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(i2c)

# target: choose the target platform
set(APP_HW_TARGET           xk-voice-l71.xn)

# Includes and flags
set(APP_INCLUDES            src ../shared)

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
