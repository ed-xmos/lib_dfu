set(LIB_NAME lib_dfu)

set(LIB_VERSION 1.1.0)

# Disable data partition dependency for DFU library, for now
set(NO_DATA_PARTITION ON)

set(LIB_DEPENDENT_MODULES "lib_xassert(4.3.2)" "lib_logging(3.4.0)")

set(LIB_INCLUDES api src)

set(LIB_XC_SRCS src/dfu_buffer_converter.xc src/dfu.xc)

if (DFU_FLASH_UNIT_TEST)
    message(STATUS "Building DFU with flash unit test stubs")
    set(LIB_XC_SRCS src flash/test/)
    list(APPEND LIB_XC_SRCS flash/test/dfu_flash_stubs.xc)
else()
    if (NO_DATA_PARTITION)
        list(APPEND LIB_XC_SRCS flash/quad/dfu_flash.xc)
    else()
        # TODO: Update lib_flash_data_partition module version... 2.5.0?
        list(APPEND LIB_DEPENDENT_MODULES "lib_flash_data_partition(2.2.0)")
        list(APPEND LIB_XC_SRCS flash/partition/dfu_flash.xc)
    endif()
endif()

# -mcmodel=large? 
set(LIB_COMPILER_FLAGS -Os -g -report -lquadflash -Wall -Wextra)

XMOS_REGISTER_MODULE()
