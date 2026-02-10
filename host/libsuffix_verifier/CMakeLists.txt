cmake_minimum_required(VERSION 3.21)

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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_LIST_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/lib)

add_library(suffix_verifier STATIC ${SOURCE_FILES})

if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(suffix_verifier PUBLIC /O2 /W4)
else()
        target_compile_options(suffix_verifier PUBLIC -g -O2 -Wall -Wextra -Wpedantic
                                -Wconversion
                                -Wdiv-by-zero
                                -Wfloat-equal
                                -Wsign-compare
                                -Wshadow)
endif()

target_include_directories(suffix_verifier PUBLIC ${INCLUDE_DIRS})
