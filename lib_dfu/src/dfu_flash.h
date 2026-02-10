// Copyright 2019-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
//
// The original intention is for those functions to be the ones used by the
// DFU implementation, and those only
//
#ifndef __dfu_flash_h__
#define __dfu_flash_h__

#include <xccompat.h>

#define _Bool int
#include <stdbool.h>

/** Possible Flash API return values */
enum flash_status {
  DFU_FLASH_BUSY = 1,
  DFU_FLASH_OK = 0,
  DFU_FLASH_OPEN_ERROR = -1,
  DFU_FLASH_ERASE_ERROR = -2,
  DFU_FLASH_GET_FACTORY_IMAGE_FAILED = -3,
  DFU_FLASH_READ_NO_IMAGE = -4,
  DFU_FLASH_BAD_PARAM = -5,
  DFU_FLASH_READ_ERROR = -6,
  DFU_FLASH_WRITE_ERROR = -7,
};

/* User flash pins config and must call fl_connect()
 * \retval DFU_FLASH_OK on success
 * \retval DFU_FLASH_OPEN_ERROR on failure
 */
enum flash_status flash_cmd_enable_ports();

/* User flash pins de-config and must call fl_disconnect()
 * \retval DFU_FLASH_OK on success
 * \retval DFU_FLASH_OPEN_ERROR on failure
 */
enum flash_status flash_cmd_disable_ports();

/** TBC */
void DFUCustomFlashEnable();

/** TBC */
void DFUCustomFlashDisable();

/** Initialise Flash sub-system
 * \retval DFU_FLASH_OK on success
 * \retval DFU_FLASH_OPEN_ERROR on failure to open flash device
 */
enum flash_status flash_cmd_init(void);

/** De-initialise Flash sub-system
 * \retval DFU_FLASH_OK on success
 * \retval DFU_FLASH_OPEN_ERROR on failure to open flash device
 */
enum flash_status flash_cmd_deinit(void);

/** Erase flash sector asynchronously
 *
 * \note Must call flash_cmd_init() before this function, and flash_cmd_deinit() when done with flash operations.
 *
 * \note This function initiates a sector erase operation and returns immediately. The caller should repeatedly call
 * this function until it reports OK or ERROR to check when the erase operation has completed.
 *
 * \param address Address in the sector to erase
 * \retval DFU_FLASH_OK if erase completed successfully
 * \retval DFU_FLASH_BUSY if erase is currently in progress
 * \retval DFU_FLASH_ERASE_ERROR if erase failed
 */
enum flash_status flash_erase_sector_async(unsigned address);

/** Write a page to flash
 * \note Must call flash_cmd_init() before this function, and flash_cmd_deinit() when done with flash operations.
 *
 * \param page Buffer containing the page data to write. The size of the page is flash_get_page_size().
 * \param length Length of the data in bytes. Must be equal to flash_get_page_size().
 *
 * \retval DFU_FLASH_OK if write completed successfully
 * \retval DFU_FLASH_BAD_PARAM if parameters are invalid (eg null pointer or wrong length)
 * \retval DFU_FLASH_ERASE_ERROR if write failed due to flash image region not being erased
 * \retval DFU_FLASH_WRITE_ERROR if write failed due to other reason
 */
enum flash_status flash_write_page(const unsigned char page[], int length);

/** Finalise write operation
 * \note Must call flash_cmd_init() before this function, and flash_cmd_deinit() when done with flash operations.
 *
 * This function should be called after all pages have been written with flash_write_page() to finalise the write
 * operation.
 * 
 * \retval DFU_FLASH_OK on success, upgrade image is now valid
 * \retval DFU_FLASH_WRITE_ERROR on failure
 */
enum flash_status flash_finalise_write();

/** Prepare to read from flash
 * \note Must call flash_cmd_init() before this function, and flash_cmd_deinit() when done with flash operations.
 * \retval DFU_FLASH_OK if ready to read
 * \retval DFU_FLASH_READ_NO_IMAGE if there is no valid upgrade image to read
 * \retval DFU_FLASH_READ_ERROR if failed to prepare for read due to other reason
 */
enum flash_status flash_start_read();

/** Read a page from flash
 * \note Must call flash_cmd_init() before this function, and flash_cmd_deinit() when done with flash operations.
 *
 * \param data Buffer to read the page data into. The size of the page is flash_get_page_size().
 * \param length Length of the data in bytes. Must be equal to flash_get_page_size().
 *
 * \retval DFU_FLASH_OK if read completed successfully
 * \retval DFU_FLASH_BAD_PARAM if parameters are invalid (eg null pointer or wrong length)
 * \retval DFU_FLASH_READ_NO_IMAGE if there is no valid upgrade image to read
 * \retval DFU_FLASH_READ_ERROR if read failed due to other reason
 */
enum flash_status flash_read_page(REFERENCE_PARAM(unsigned char, data), int length);

/** Check if flash is busy with an operation
 * \retval true if flash is busy
 * \retval false if flash is not busy
 */
bool flash_is_busy(void);

/** Get the size of a flash page in bytes
 * \return page size in bytes
 */
int flash_get_page_size(void);

/** Get the total size of the flash in bytes
 * \return flash size in bytes
 */
int flash_get_size(void);

/* The following is DEPRECATED
 * Will be removed as dfu.xc is updated.
 */

#include "dfu_flash_result.h"

bool flash_is_first_whole_page_in_sector(unsigned address);

bool flash_is_sector_erased(unsigned address);

enum flash_locate_boot_upgrade_slot_result flash_locate_boot_upgrade_slot(REFERENCE_PARAM(unsigned, address));

enum flash_locate_data_upgrade_slot_result flash_locate_data_upgrade_slot(REFERENCE_PARAM(unsigned, address));

enum flash_set_write_disable_result flash_set_write_disable(void);

bool flash_verify_page(unsigned address, const char page[]);

int flash_get_data_partition_base(void);

void flash_cmd_read_page(unsigned char *data);

#endif
