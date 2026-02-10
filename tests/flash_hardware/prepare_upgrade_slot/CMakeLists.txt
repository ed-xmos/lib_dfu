cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(test_prepare_upgrade_slot)

set(APP_HW_TARGET          XK-EVK-XU316)
# set(APP_HW_TARGET          XK_VOICE_L71.xn)

set(XMOS_SANDBOX_DIR       ${CMAKE_CURRENT_LIST_DIR}/../../../..)

set(APP_DEPENDENT_MODULES  "lib_dfu" "lib_unity(2.6.1)")

# set(DFU_FLASH_UNIT_TEST OFF)

# -march=xs3a -mcmodel=large ??
set(COMPILER_FLAGS      -g
                        -Wno-xcore-fptrgroup
                        -fcmdline-buffer-bytes=256
                        -lquadflash
                        -mno-dual-issue
                        -report
                        -Wall 
                        -Wextra
                        -Wshadow
                        -Wconversion
                        -Wdiv-by-zero
                        -Wfloat-equal
                        -Wsign-compare)

# # add uut and test sources
file(GLOB APP_C_SRCS CONFIGURE_DEPENDS RELATIVE ${CMAKE_CURRENT_LIST_DIR} src/*)
set(APP_INCLUDES src)

# Create config for each test
file(GLOB tests RELATIVE ${CMAKE_CURRENT_LIST_DIR} CONFIGURE_DEPENDS src/test*.c)
foreach(test_file ${tests})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(SOURCE_FILES_${test_name}       ${test_file})
    set(APP_COMPILER_FLAGS_${test_name} ${COMPILER_FLAGS})
endforeach()

# Enable auto gen of test runners
set(LIB_UNITY_AUTO_TEST_RUNNER OFF)

# Disable unity features that we dont use
set(LIB_UNITY_USE_FIXTURE OFF)
set(LIB_UNITY_USE_MEMORY OFF)

XMOS_REGISTER_APP()

message("Build targets: ${APP_BUILD_TARGETS}")