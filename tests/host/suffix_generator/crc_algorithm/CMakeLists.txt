cmake_minimum_required(VERSION 3.21)

project(crc_algorithm_test)

# Force release mode so that all the DLLs are statically linked
set (CMAKE_BUILD_TYPE "Release" CACHE STRING "Only Release mode is allowed" FORCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_LIST_DIR}/bin)

add_executable(crc_algorithm crc_algorithm.c ../../../../host/suffix_generator/src/crc.c)

target_include_directories(crc_algorithm PRIVATE ../../../../host/suffix_generator/src)


if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(crc_algorithm PRIVATE /O2 /W4)
else()
    target_compile_options(crc_algorithm PRIVATE -g -O2 -Wall -Wextra -Wpedantic
                                -Wconversion
                                -Wdiv-by-zero
                                -Wfloat-equal
                                -Wsign-compare
                                -Wshadow)
endif()
