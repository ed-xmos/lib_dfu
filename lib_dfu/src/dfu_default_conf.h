// Copyright 2011-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef DFU_DEFAULT_CONF_H
#define DFU_DEFAULT_CONF_H

#ifdef __dfu_conf_h_exists__
#warning "Custom dfu_conf.h found, default configuration will be overridden"
#include "dfu_conf.h"
#endif

#ifdef __DOXYGEN__
/** User defined flash device specification for DFU to use.
 * 
 * If not defined, the default flash device list in dfu_flashlib_user.c is used.
 * This should be defined as a fl_DeviceSpec or fl_QuadDeviceSpec structure,
 * depending on whether DFU_QUAD_SPI_FLASH is set.
 */
#define DFU_USER_FLASH_DEVICE
#endif

/** Whether DFU uses Quad SPI Flash, or older SPI flash (when 0) */
#ifndef DFU_QUAD_SPI_FLASH
#define DFU_QUAD_SPI_FLASH 1
#endif

/** Number of bytes transferred in each DFU packet on the given physical transport */
#ifndef DFU_TRANSFER_SIZE_BYTES
#define DFU_TRANSFER_SIZE_BYTES 64
#endif

/** Number of bytes in a flash page for the target device */
#ifndef DFU_FLASH_PAGE_SIZE_BYTES
#define DFU_FLASH_PAGE_SIZE_BYTES 256
#endif

/** Number of DFU packets per flash page */
#ifndef NUM_DFU_PAGES_PER_FLASH_PAGE
#define NUM_DFU_PAGES_PER_FLASH_PAGE (DFU_FLASH_PAGE_SIZE_BYTES / DFU_TRANSFER_SIZE_BYTES)
#if DFU_TRANSFER_SIZE_BYTES > DFU_FLASH_PAGE_SIZE_BYTES
#error "DFU_TRANSFER_SIZE_BYTES must not be greater than DFU_FLASH_PAGE_SIZE_BYTES"
#endif
#endif

/* TODO - can we use the DFU image size from download or block 0? 
 * And remove this or convert it to a ceiling value, sensible max rather than actual erase size */

/* Defines flash area to erase on first DFU download request received
 *
 * Flash library will round it up to the nearest sector, e.g. 4KB
 *
 * XS2 internal flash IS25LQ016B takes 70ms to erase one sector
 * 128KB will take over 2 seconds, for instance
 *
 * Your host software might implement a 5sec timeout as per USB spec 9.2.6.1,
 * and 5 seconds is just over 300KB
 */
#ifndef FLASH_MAX_UPGRADE_SIZE
#define FLASH_MAX_UPGRADE_SIZE (512 * 1024)
#endif

#endif /* DFU_DEFAULT_CONF_H */
