// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef DFU_USB_REQUESTS_H
#define DFU_USB_REQUESTS_H

#include <xccompat.h>

#include "dfu.h"
#if defined(DFU_USB_EN) && DFU_USB_EN

#include "xud_device.h"
#include "dfu_interface.h"

// TODO - refactor to remove "DFU_mode_active" param.

/** Check if DFU mode is active
 * Returns 1 if DFU mode is active, 0 if DFU mode is not active.
 */
int DFUModeIsActive(void);

/** Set DFU mode as active */
void DFUSetModeActive(void);

/** Set DFU mode as inactive */
void DFUSetModeInactive(void);

/** Check the initial state of DFU mode
 * Used during boot to determine whether to enter DFU mode or not.
 * 
 * \todo update parameter to user customisable.
 */
void DFUCheckInitState(NULLABLE_RESOURCE(chanend, c_aud_ctl));

/* Handle USB reset events
 * 
 * Returns 1 if the device should be in DFU mode, 0 if it should be in application mode.
 */
int DFUProcessResetState(CLIENT_INTERFACE(i_dfu, i));

/* Helper function for C */
void DFUDelay(unsigned d);

/* Handle XMOS specific DFU requests
 * 
 * Returns XUD_RES_OKAY if request was handled, XUD_RES_ERR if request was not recognised/handled.
 */
int dfu_usb_vendor_requests(XUD_ep ep0_out, XUD_ep ep0_in, REFERENCE_PARAM(USB_SetupPacket_t, sp), CLIENT_INTERFACE(i_dfu, dfuInterface), unsigned int xua_dfu_interface_num);

/* Handle standard DFU requests
 *
 * Returns XUD_RES_OKAY if request was handled, XUD_RES_ERR if request was not recognised/handled.
 */
int dfu_usb_class_int_requests(XUD_ep ep0_out, XUD_ep ep0_in, REFERENCE_PARAM(USB_SetupPacket_t, sp), CLIENT_INTERFACE(i_dfu, dfuInterface), NULLABLE_RESOURCE(chanend, c_aud_ctl), unsigned int xua_dfu_interface_num);

/* User callback for notification of entry to DFU mode. */
// TODO - make parameter user customisable via macro or something, and remove from DFU interface since this is really a user callback and not a DFU interface function
void DFUNotifyEntryCallback(NULLABLE_RESOURCE(chanend, c_aud_ctl), int handshake);

#endif /* DFU_USB_EN */
#endif /* DFU_USB_REQUESTS_H */
