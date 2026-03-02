cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(i2c)

# target: choose the target platform
set(APP_HW_TARGET           xk-voice-l71.xn)

# Includes and flags
set(APP_INCLUDES            src ../shared)

set(COMPILER_FLAGS          -O2
                            -g
                            -lquadflash
                            -report
                            -Wall
                            -Wextra
)

set(APP_COMPILER_FLAGS_factory  ${COMPILER_FLAGS} -DDFU_BCD_DEVICE=0x0101)
set(APP_COMPILER_FLAGS_update   ${COMPILER_FLAGS} -DDFU_BCD_DEVICE=0x0200)

include(../deps.cmake)

set(XMOS_SANDBOX_DIR        ${CMAKE_CURRENT_LIST_DIR}/../../../..)

XMOS_REGISTER_APP()
