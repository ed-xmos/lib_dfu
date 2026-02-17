cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(test_dfu_dnload)

set(APP_HW_TARGET           XK-EVK-XU316)

# Removed lib_dfu from the dependent modules as we are adding the source files directly to this test app.
set(APP_DEPENDENT_MODULES   "lib_unity(2.6.1)" lib_xassert lib_logging)

set(DFU_FLASH_UNIT_TEST ON)

set(COMPILER_FLAGS      -O3
                        -report
                        -Wall
                        -Wextra
                        -Wshadow
                        -Wconversion
                        -Wdiv-by-zero
                        -Wfloat-equal
                        -Wsign-compare)

set(LIB_DFU_INCLUDE_PATH    ${CMAKE_CURRENT_LIST_DIR}/../../../../lib_dfu)
set(APP_INCLUDES            src ${LIB_DFU_INCLUDE_PATH}/api ${LIB_DFU_INCLUDE_PATH}/src ${LIB_DFU_INCLUDE_PATH}/src/modules)

set(XMOS_SANDBOX_DIR    ${CMAKE_CURRENT_LIST_DIR}/../../../../..)

# add uut and test sources
file(GLOB APP_C_SRCS CONFIGURE_DEPENDS RELATIVE ${CMAKE_CURRENT_LIST_DIR} src/*)
list(APPEND APP_C_SRCS  ../../../../lib_dfu/src/modules/fifo.c)
set(APP_XC_SRCS         ../../../../lib_dfu/src/dfu.xc)

# Create config for each test
file(GLOB tests RELATIVE ${CMAKE_CURRENT_LIST_DIR} CONFIGURE_DEPENDS src/test*.c)
foreach(test_file ${tests})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(SOURCE_FILES_${test_name} ${test_file})
    set(APP_COMPILER_FLAGS_${test_name} ${COMPILER_FLAGS})
endforeach()

# Enable auto gen of test runners
set(LIB_UNITY_AUTO_TEST_RUNNER ON)

# Disable unity features that we dont use
set(LIB_UNITY_USE_FIXTURE OFF)
set(LIB_UNITY_USE_MEMORY OFF)

XMOS_REGISTER_APP()
