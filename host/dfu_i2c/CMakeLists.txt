cmake_minimum_required(VERSION 3.12)

project("dfu_i2c")

# Force release mode so that all the DLLs are statically linked
set (CMAKE_BUILD_TYPE "Release" CACHE STRING "Only Release mode is allowed" FORCE)

add_compile_definitions(_GNU_SOURCE HOST_APP)

include(../../../lib_device_control/host/host_build_i2c.cmake)

set (INCLUDE_DIRS
        "../api"
        "../../lib_dfu/api"
        "../../lib_dfu/src"
        "../libsuffix_verifier/src"
        "src"
)

set (SOURCE_FILES
        src/argument_parser.c
        src/hal.c
        src/input_reader.c
        src/operations.c
        src/dfu_utils.c
        src/labels.c
        ../libsuffix_verifier/src/crc.c
        ../libsuffix_verifier/src/suffix_verifier.c
)

set (DFUCTRL_LIB dfuctrl_i2c_1.0)
set (DFUCTRL_APP dfu_i2c)
add_compile_definitions(nologo CONTROL_USE_I2C CONTROL_RPI)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
set(FLAGS /O2 /W4)
else()
set(FLAGS 
            -g
            -O2
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wdiv-by-zero
            -Wfloat-equal
            -Wsign-compare
            -Wshadow)
endif()


add_library(${DFUCTRL_LIB} STATIC ${SOURCE_FILES})
target_include_directories(${DFUCTRL_LIB} PUBLIC ${INCLUDE_DIRS})
target_compile_options(${DFUCTRL_LIB} PUBLIC ${FLAGS})
target_link_libraries(${DFUCTRL_LIB} control_i2c_host)

add_executable(${DFUCTRL_APP} src/main.c)
target_include_directories(${DFUCTRL_APP} PUBLIC "api")
target_compile_options(${DFUCTRL_APP} PUBLIC ${FLAGS})
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(${DFUCTRL_APP} ${DFUCTRL_LIB})
else()
    target_link_libraries(${DFUCTRL_APP} ${DFUCTRL_LIB} m)
endif()

