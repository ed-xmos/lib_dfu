cmake_minimum_required(VERSION 3.21)

project(host)

add_subdirectory(libsuffix_verifier)
add_subdirectory(suffix_generator)
add_subdirectory(xmosdfu)
add_subdirectory(dfu_i2c)
