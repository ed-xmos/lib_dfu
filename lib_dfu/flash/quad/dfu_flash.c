// Copyright 2019-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

// WIP NOTICE:  The code below has been commented out to allow successful compilation.
//              It requires further work to reinstate the functionality.

#include "dfu_flash.h"

#include <quadflashlib.h>
#include <safestring.h>

#include "dfu.h"

struct flash_session {
  int device_open;
  fl_BootImageInfo factory_image;
  fl_BootImageInfo upgrade_image;

  int upgrade_image_valid;
};

static struct flash_session session;

enum flash_status flash_cmd_enable_ports() __attribute__((weak));
enum flash_status flash_cmd_enable_ports() { return DFU_FLASH_OPEN_ERROR; }

enum flash_status flash_cmd_disable_ports() __attribute__((weak));
enum flash_status flash_cmd_disable_ports() { return DFU_FLASH_OPEN_ERROR; }

// TODO move to central location
void DFUCustomFlashEnable() __attribute__((weak));
void DFUCustomFlashEnable() {}

void DFUCustomFlashDisable() __attribute__((weak));
void DFUCustomFlashDisable() {}

/* Returns non-zero for error */
enum flash_status flash_cmd_init(void) {
  fl_BootImageInfo image;

  if (!session.device_open) {
    if (flash_cmd_enable_ports() == DFU_FLASH_OK) {
      session.device_open = 1;
    }
  }

  if (!session.device_open) {
    return DFU_FLASH_OPEN_ERROR;
  }

#if defined(DFU_QUAD_SPI_FLASH) && (DFU_QUAD_SPI_FLASH == 0)
  // Disable flash protection
  fl_setProtection(0);
#endif

  if (fl_getFactoryImage(&image) != 0) {
    return DFU_FLASH_GET_FACTORY_IMAGE_FAILED;
  }

  session.factory_image = image;

  if (fl_getNextBootImage(&image) == 0) {
    session.upgrade_image_valid = 1;
    session.upgrade_image = image;
  }

  return DFU_FLASH_OK;
}

enum flash_status flash_cmd_deinit(void) {
  if (!session.device_open) {
    return DFU_FLASH_OK;
  }

  flash_cmd_disable_ports();
  session.device_open = 0;
  return DFU_FLASH_OK;
}

// int flash_cmd_start_write_image()
enum flash_status flash_erase_sector_async(unsigned address) {
  (void)address;

  int ret = 0;
  if (session.upgrade_image_valid) {
    ret = fl_startImageReplace(&session.upgrade_image, FLASH_MAX_UPGRADE_SIZE);
  } else {
    ret = fl_startImageAdd(&session.factory_image, FLASH_MAX_UPGRADE_SIZE, 0);
  }
  if (ret < 0) {
    return DFU_FLASH_ERASE_ERROR;
  } else if (ret > 0) {
    return DFU_FLASH_BUSY;
  } else {
    session.upgrade_image_valid = 0;
    return DFU_FLASH_OK;
  }
}

enum flash_status flash_write_page(const unsigned char *page, int length) {
  if (page == NULL || length != (int)fl_getPageSize()) {
    return DFU_FLASH_BAD_PARAM;

  } else if (session.upgrade_image_valid) {
    return DFU_FLASH_ERASE_ERROR;

  } else if (fl_writeImagePage(page) != 0) {
    return DFU_FLASH_WRITE_ERROR;
  }
  return DFU_FLASH_OK;
}

enum flash_status flash_finalise_write() {
  if (fl_endWriteImage() != 0) {
    return DFU_FLASH_WRITE_ERROR;
  }

  // Sanity check
  fl_BootImageInfo image = session.factory_image;
  if (fl_getNextBootImage(&image) != 0) {
    return DFU_FLASH_OPEN_ERROR;
  }
  session.upgrade_image = image;
  session.upgrade_image_valid = 1;
  return DFU_FLASH_OK;
}

enum flash_status flash_start_read() {
  if (!session.upgrade_image_valid) {
    return DFU_FLASH_READ_NO_IMAGE;

  } else {
    int read = fl_startImageRead(&session.upgrade_image);
    if (read != 0) {
      return DFU_FLASH_READ_ERROR;
    } else {
      return DFU_FLASH_OK;
    }
  }
}

enum flash_status flash_read_page(unsigned char *data, int length) {
  if (data == NULL || length != (int)fl_getPageSize()) {
    return DFU_FLASH_BAD_PARAM;

  } else if (!session.upgrade_image_valid) {
    return DFU_FLASH_READ_NO_IMAGE;
  }

  if (fl_readImagePage(data) != 0) {
    return DFU_FLASH_READ_ERROR;
  }
  return DFU_FLASH_OK;
}

bool flash_is_busy(void) { return (fl_getBusyStatus() != 0); }

int flash_get_page_size(void) { return (int)fl_getPageSize(); }

int flash_get_size(void) { return (int)fl_getFlashSize(); }

#include "dfu_flash_result.h"

// TEMP - "extra"
// int fl_getSectorEndAddress(int sectorNum);
// void fl_int_eraseSector(unsigned char cmd, unsigned int sectorAddress);
// int fl_getSectorContaining(unsigned address);

enum flash_locate_boot_upgrade_slot_result flash_locate_boot_upgrade_slot(unsigned *address) {
  (void)address;
  return FLASH_LOCATE_BOOT_UPGRADE_SLOT_SUCCESS;
}

enum flash_locate_data_upgrade_slot_result flash_locate_data_upgrade_slot(unsigned *address) {
  (void)address;
  return FLASH_LOCATE_DATA_UPGRADE_SLOT_SUCCESS;
}

bool flash_is_first_whole_page_in_sector(unsigned address) {
  unsigned page_size = fl_getPageSize();

  if (address < page_size) {
    return true;
  }

  return false;  // TODO Fix
}

bool flash_is_sector_erased(unsigned address) {
  (void)address;
  return true;
}

enum flash_set_write_disable_result flash_set_write_disable(void) { return FLASH_SET_WRITE_DISABLE_ERROR; }

bool flash_verify_page(unsigned address, const char page[]) {
  (void)address;
  unsigned int page_size = fl_getPageSize();
  unsigned char verify[DFU_FLASH_PAGE_SIZE_BYTES];

  fl_readImagePage(verify);
  return safememcmp(verify, (const unsigned char *)page, page_size) == 0;
}

// No data partition support
int flash_get_data_partition_base(void) { return -1; }

void flash_cmd_read_page(unsigned char *data) {
  *(unsigned int *)data = 1;
  return;
}
