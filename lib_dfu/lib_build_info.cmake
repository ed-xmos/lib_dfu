set(LIB_NAME lib_dfu)

set(LIB_VERSION 1.1.0)

# Disable data partition dependency for DFU library, for now
set(NO_DATA_PARTITION ON)

set(LIB_DEPENDENT_MODULES   "lib_xassert(4.3.2)" "lib_logging(3.4.0)")

set(LIB_INCLUDES            api src)

set(LIB_C_SRCS              src/dfu_flashlib_user.c)
set(LIB_XC_SRCS             src/dfu_buffer_converter.xc src/dfu.xc)

if (DFU_FLASH_UNIT_TEST)
    message(STATUS "Building DFU with flash unit test stubs")
    list(APPEND LIB_C_SRCS flash/test/dfu_flash_stubs.c)
else()
    if (NO_DATA_PARTITION)
        list(APPEND LIB_C_SRCS flash/quad/dfu_flash.c)
    else()
        # TODO: Update lib_flash_data_partition module version... 2.5.0?
        list(APPEND LIB_DEPENDENT_MODULES "lib_flash_data_partition(2.2.0)")
        list(APPEND LIB_C_SRCS flash/partition/dfu_flash.c)
    endif()
endif()

# -mcmodel=large? 
set(LIB_COMPILER_FLAGS      -Os -g -report -Wall -Wextra)

set(LIB_OPTIONAL_HEADERS    dfu_conf.h)

XMOS_REGISTER_MODULE()
