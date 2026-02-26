cmake_minimum_required(VERSION 3.12)

project("dfu_i2c")

# Force release mode so that all the DLLs are statically linked
set (CMAKE_BUILD_TYPE "Release" CACHE STRING "Only Release mode is allowed" FORCE)

add_compile_definitions(_GNU_SOURCE HOST_APP)

set (INCLUDE_DIRS 
        "../../../lib_device_control/lib_device_control/api"
        "../../../lib_device_control/lib_device_control/inc"
        "../../../lib_device_control/host/api"
        "../../../lib_device_control/host/inc"
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
        ../../../lib_device_control/host/src/control_host_util.c
)

set (LINK_LIBS)

set (DFUCTRL_LIB dfuctrl_i2c_1.0)
set (DFUCTRL_APP dfu_i2c)
add_compile_definitions(nologo USE_I2C RPI)
set (SOURCE_FILES ${SOURCE_FILES} ../../../lib_device_control/host/src/device_access_i2c_rpi.c)


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_library(${DFUCTRL_LIB} STATIC ${SOURCE_FILES})
target_compile_definitions(${DFUCTRL_LIB} PUBLIC)
target_include_directories(${DFUCTRL_LIB} PUBLIC ${INCLUDE_DIRS})
target_link_libraries(${DFUCTRL_LIB} ${LINK_LIBS})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(${DFUCTRL_APP} src/main.c)
target_include_directories(${DFUCTRL_APP} PUBLIC "api")
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(${DFUCTRL_APP} ${DFUCTRL_LIB})
else()
    target_link_libraries(${DFUCTRL_APP} ${DFUCTRL_LIB} m)
endif()

