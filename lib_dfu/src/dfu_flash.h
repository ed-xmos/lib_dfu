// Copyright 2019-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef DFU_FLASH_H
#define DFU_FLASH_H

#include <xccompat.h>
#include <stdint.h>

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

/** Structure to return flash data and status */
struct flash_data_status {
  enum flash_status status;
  int32_t data;
};

/** User flash pins config and must call fl_connect()
 * \retval DFU_FLASH_OK on success
 * \retval DFU_FLASH_OPEN_ERROR on failure
 */
enum flash_status flash_enable_ports();

/** User flash pins de-config and must call fl_disconnect()
 * \retval DFU_FLASH_OK on success
 * \retval DFU_FLASH_OPEN_ERROR on failure
 */
enum flash_status flash_disable_ports();

/** TBC */
void DFUCustomFlashEnable();

/** TBC */
void DFUCustomFlashDisable();

/** Initialise Flash sub-system
 * \retval DFU_FLASH_OK on success
 * \retval DFU_FLASH_OPEN_ERROR on failure to open flash device
 */
enum flash_status flash_init(void);

/** De-initialise Flash sub-system
 * \retval DFU_FLASH_OK on success
 * \retval DFU_FLASH_OPEN_ERROR on failure to open flash device
 */
enum flash_status flash_deinit(void);

/** Is flash connected 
 * \retval 1 if flash is connected and ready for operations
 * \retval 0 if flash is not connected or not ready for operations
 */
int32_t flash_is_connected(void);

/** Given the first page to write to flash, get the image size 
 *
 * \param buf Buffer containing the first page data.
 * \param length Length of the data in bytes. Must be equal to flash_get_page_size().
 * 
 * \retval DFU_FLASH_OK if image size was successfully read, data is valid and field contains the image size in bytes
 * \retval DFU_FLASH_BAD_PARAM if parameters are invalid
 * \retval DFU_FLASH_READ_NO_IMAGE if the buffer does not contain a valid image header with size information
 */
struct flash_data_status flash_get_image_size_from_buffer(const uint8_t buf[], int32_t length);

/** Erase flash sector asynchronously
 *
 * \note Must call flash_init() before this function, and flash_deinit() when done with flash operations.
 *
 * \note This function initiates a sector erase operation and returns immediately. The caller should repeatedly call
 * this function until it reports OK or ERROR to check when the erase operation has completed.
 *
 * \param erase_size Amount of flash memory to erase, will be converted to sector count internally.
 * \retval DFU_FLASH_OK if erase completed successfully
 * \retval DFU_FLASH_BAD_PARAM if parameters are invalid (eg erase_size is 0)
 * \retval DFU_FLASH_BUSY if erase is currently in progress
 * \retval DFU_FLASH_ERASE_ERROR if erase failed
 */
enum flash_status flash_erase_sector_async(int32_t erase_size);

/** Write a page to flash and verify, synchronous operation
 * \note Must call flash_init() before this function, and flash_deinit() when done with flash operations.
 *
 * \param page Buffer containing the page data to write. The size of the page is flash_get_page_size().
 * \param length Length of the data in bytes. Must be equal to flash_get_page_size().
 *
 * \retval DFU_FLASH_OK if write completed successfully
 * \retval DFU_FLASH_BAD_PARAM if parameters are invalid (eg null pointer or wrong length)
 * \retval DFU_FLASH_ERASE_ERROR if write failed due to flash image region not being erased
 * \retval DFU_FLASH_WRITE_ERROR if write failed due to other reason
 */
enum flash_status flash_write_page(const uint8_t page[], int32_t length);

/** Finalise write operation
 * \note Must call flash_init() before this function, and flash_deinit() when done with flash operations.
 *
 * This function should be called after all pages have been written with flash_write_page() to finalise the write
 * operation.
 * 
 * \retval DFU_FLASH_OK on success, upgrade image is now valid
 * \retval DFU_FLASH_WRITE_ERROR on failure
 */
enum flash_status flash_finalise_write();

/** Prepare to read from flash
 * \note Must call flash_init() before this function, and flash_deinit() when done with flash operations.
 * 
 * \retval DFU_FLASH_OK if ready to read, data filed is valid and contains the size of the upgrade image in bytes
 * \retval DFU_FLASH_READ_NO_IMAGE if there is no valid upgrade image to read
 * \retval DFU_FLASH_READ_ERROR if failed to prepare for read due to other reason
 */
struct flash_data_status flash_start_read();

/** Read a page from flash
 * \note Must call flash_init() before this function, and flash_deinit() when done with flash operations.
 *
 * \param data    Buffer to read the page data into. The size of the page is flash_get_page_size().
 * \param length  Length of the data buffer in bytes. Must be equal to flash_get_page_size().
 *
 * \retval DFU_FLASH_OK if read completed successfully
 * \retval DFU_FLASH_BAD_PARAM if parameters are invalid (eg null pointer or wrong length)
 * \retval DFU_FLASH_READ_NO_IMAGE if there is no valid upgrade image to read
 * \retval DFU_FLASH_READ_ERROR if read failed due to other reason, or if end-of-image reached.
 */
enum flash_status flash_read_page(uint8_t data[], int32_t length);

/** Check if flash is busy with an operation
 * \retval 1 if flash is busy
 * \retval 0 if flash is not busy
 */
int32_t flash_is_busy(void);

/** Get the size of a flash page in bytes
 * \return page size in bytes
 */
int32_t flash_get_page_size(void);

/** Get the size of a flash sector in bytes
 * \return sector size in bytes
 */
int32_t flash_get_sector_size(void);

/** Get the total size of the flash in bytes
 * \return flash size in bytes
 */
int32_t flash_get_size(void);

/**
 * Perform sanity checks of flash device about to be used for firmware upgrade
 *
 * Caller chooses when to call this. Comprises a set of assertions to verify
 * that the implementation relies on.
 *
 * Flash connection is necessary
 *
 * \return Whether specification is suitable for use by this library
 */
int32_t flash_is_suitable(void);

#endif
