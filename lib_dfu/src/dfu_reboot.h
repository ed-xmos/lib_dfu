// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef DFU_REBOOT_H
#define DFU_REBOOT_H

#include "dfu.h"

#if (DFU_ENABLE == 1)
/* Similarly to the delay before reboot to DFU mode, this delay is meant to
 * avoid shocking the Windows software stack. Suggest revisiting to establish
 * if 50 or 500 is needed.
 */
#define DELAY_BEFORE_REBOOT_FROM_DFU_MS   50
#else

/* TESTING */
#define DELAY_BEFORE_REBOOT_FROM_DFU_MS   1

#endif

/* Reboot the device */
void device_reboot(void);


#endif /* DFU_REBOOT_H */
