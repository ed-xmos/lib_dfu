# This file provides overrides to the Doxyfile configuration

PROJECT_NAME            = lib_dfu
PROJECT_BRIEF           = "Device Firmware Update (DFU)"

INPUT                   = ../lib_dfu/api ../lib_dfu/src ../host ../lib_dfu/flash

PREDEFINED              = C_API= EXTERN_C= slave= __DOXYGEN__=1 interface= __attribute__(x)=
