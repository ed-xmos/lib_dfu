cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(host_i2c_xcore)

# root
set(XMOS_SANDBOX_DIR ${CMAKE_SOURCE_DIR}/../../../..)

include("${XMOS_SANDBOX_DIR}/lib_device_control/host/host_build_i2c.cmake")

# target: choose the target platform
set(APP_HW_TARGET           XK-EVK-XU316)

set(APP_INCLUDES            src ../shared)

set(APP_COMPILER_FLAGS      -O2
                            -g
                            -report
                            -Wall
                            -Wextra
)

include(../deps.cmake)

XMOS_REGISTER_APP()

target_link_libraries(${PROJECT_NAME} PRIVATE control_i2c_host)

