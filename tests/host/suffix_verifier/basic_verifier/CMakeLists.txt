cmake_minimum_required(VERSION 3.21)

project(basic_verifier_test)

# Force release mode so that all the DLLs are statically linked
set (CMAKE_BUILD_TYPE "Release" CACHE STRING "Only Release mode is allowed" FORCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_LIST_DIR}/bin)

set(SUFFIX_VERIFIER ../../../../host/libsuffix_verifier)

add_executable(basic_verifier basic_verifier.c ${SUFFIX_VERIFIER}/src/crc.c)

target_include_directories(basic_verifier PRIVATE ${SUFFIX_VERIFIER}/src)

if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(basic_verifier PRIVATE /O2 /W4)
else()
    target_compile_options(basic_verifier PRIVATE -g -O2 -Wall -Wextra -Wpedantic
                                -Wconversion
                                -Wdiv-by-zero
                                -Wfloat-equal
                                -Wsign-compare
                                -Wshadow)
endif()

target_link_libraries(basic_verifier PRIVATE suffix_verifier)
target_link_directories(basic_verifier PRIVATE ${SUFFIX_VERIFIER}/lib)
