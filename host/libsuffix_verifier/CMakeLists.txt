cmake_minimum_required(VERSION 3.7)

project(suffix_verifier C)

# Force release mode so that all the DLLs are statically linked
set (CMAKE_BUILD_TYPE "Release" CACHE STRING "Only Release mode is allowed" FORCE)

# set (DEFINES _GNU_SOURCE HOST_APP)

set (INCLUDE_DIRS 
        "../api"
)

set (SOURCE_FILES
        src/suffix_verifier.c
        src/crc.c
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/../bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)

add_library(suffix_verifier STATIC ${SOURCE_FILES})

target_compile_options(suffix_verifier PUBLIC -g -O2 -Wall -Wextra -Wpedantic
                            -Wconversion
                            -Wdiv-by-zero
                            -Wfloat-equal
                            -Wsign-compare)

target_include_directories(suffix_verifier PUBLIC ${INCLUDE_DIRS})