/*
 * Quark Microcontroller DFU Utility
 * Copyright (C) 2016, Intel Corporation
 * 
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 only, as published by
 * the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef _QDA_H_
#define _QDA_H_

#include <stdint.h>
#include "usb_dfu.h"

#include "qda_packets.h"

/* Note: dfu_status and dfu_if are copied from dfu.h */

/**
 * DFU status structure
 */
typedef struct dfu_status {
	unsigned char bStatus;
	unsigned int bwPollTimeout;
	unsigned char bState;
	unsigned char iString;
} dfu_status_t;

/**
 * QDA interface structure
 */
typedef struct dfu_if {
	struct usb_dfu_func_descriptor func_dfu;
	uint16_t vendor;
	uint16_t product;
	uint16_t bcdDevice;
	uint8_t interface;
	uint8_t altsetting;
	uint8_t bMaxPacketSize0;
} qda_if_t;

/**
 * QDA Configuration structure.
 */
typedef struct qda_conf_s {
	/**
	 * QDA send callback.
	 *
	 * @param[in] data Data to be sent.
	 * @param[in] len Length of data to be sent.
	 *
	 * @return Error Status
	 * @retval 0 Success
	 * @retval -1 Error
	 */
	int (*send)(uint8_t *data, size_t len);

	/**
	 * QDA receive callback.
	 *
	 * @param[out] data Data buffer for received bytes.
	 * @param[in] len Maximum allocated bytes that can be received.
	 *
	 * @return Length of received data.
	 */
	size_t (*receive)(uint8_t *data, size_t len);

	/**
	 * QDA detach callback.
	 *
	 * @return Error Status
	 * @retval 0 Success
	 * @retval -1 Error
	 **/
	int (*detach)(void);
} qda_conf_t;

/**
 * Init QDA and set configuration.
 *
 * @paqram[in] conf A QDA configuration structure.
 *
 * @return Error Status
 * @retval 0 Success
 * @retval -1 Error
  */
int qda_init(qda_conf_t *conf);

/**
 * Reset QDA device.
 *
 * @return Error Status
 * @retval 0 Success
 * @retval -1 Error
 */
int qda_reset();

/**
 * Get device description.
 *
 * @param[out] dif Target structure to update configuration values.
 *
 * @return Error Status
 * @retval 0 Success
 * @retval -1 Error
 */
int qda_get_dev_desc(qda_if_t *dif);

/**
 * Get DFU description.
 *
 * @param[out] dif Target structure to update configuration values.
 *
 * @return Error Status
 * @retval 0 Success
 * @retval -1 Error
 */
int qda_get_dfu_desc(qda_if_t *dif);

/**
 * Set alternate setting.
 *
 * @param[in] alt Alternate setting to be set.
 *
 * @return Error Status
 * @retval 0 Success
 * @retval -1 Error
 */
int qda_set_alt_setting(uint8_t alt);

/**
 * Detach device and enter DFU mode.
 *
 * @return Error Status
 * @retval 0 Success
 * @retval -1 Error
 */
int qda_dfu_detach(void);

/**
 * Perform a DFU download.
 *
 * Write firmware data to the target device.
 *
 * @param[in] len Length of block.
 * @param[in] transaction Block number.
 * @param[in] data Pointer to data.
 *
 * @return Error Status
 * @retval 0 Success
 * @retval -1 Error
 */
int qda_dfu_download(uint16_t len, uint16_t transaction, const uint8_t *data);

/**
 * Perform a DFU upload.
 *
 * Read firmware data from target device.
 *
 * @param[in] len Length of block.
 * @param[in] transaction Block number.
 * @param[out] data Pointer to data.
 *
 * @return Length or Error Status
 * @retval >0 Length of downloaded data.
 * @retval 0 Success
 * @retval -1 Error
 */
int qda_dfu_upload(uint16_t len, uint16_t transaction, uint8_t *data);

/**
 * Request device's DFU status.
 *
 * @param[out] status The current DFU status.
 *
 * @return Error Status
 * @retval 0 Success
 * @retval -1 Error
 */
int qda_dfu_getstatus(dfu_status_t *status);

/**
 * Clear device's DFU status.
 *
 * @return Error Status
 * @retval 0 Success
 * @retval -1 Error
 */
int qda_dfu_clrstatus();

/**
 * Request device's DFU state.
 *
 * @return Success or error State
 * @retval >=0 DFU State
 * @retval -1 Error
 */
int qda_dfu_getstate();

/**
 * Send DFU abort message to device.
 *
 * @return Error Status
 * @retval 0 Success
 * @retval -1 Error
 */
int qda_dfu_abort();

/* Not implemented */
const char *qda_dfu_state_to_string(int state);

/**
 * Retrieve status string.
 *
 * Return status string of a valid DFU status.
 *
 * @param[in] status The DFU status number.
 *
 * @return String of the requested status number.
 */
const char *qda_dfu_status_to_string(int status);

/* DFU SHIM */
#define dfu_detach(dev, interface, timeout) qda_dfu_detach()
#define dfu_download(dev, interface, len, trans, data)                         \
	qda_dfu_download(len, trans, data)
#define dfu_upload(dev, interface, len, trans, data)                           \
	qda_dfu_upload(len, trans, data)

#define dfu_get_status(dif, status) qda_dfu_getstatus(status)
#define dfu_clear_status(dev, interface) qda_dfu_getstate()
#define dfu_abort(dev, interface) qda_dfu_abort()
#define dfu_state_to_string(state) qda_dfu_state_to_string(state)
#define dfu_status_to_string(status) qda_dfu_status_to_string(status)

/* LIBUSB SHIM */
#define libusb_set_interface_alt_setting(handle, interface, alt)               \
	qda_set_alt_setting(alt)
#define libusb_reset_device(ndle) qda_reset()

static inline uint16_t libusb_cpu_to_le16(const uint16_t x)
{
	union {
		uint8_t b8[2];
		uint16_t b16;
	} _tmp;
	_tmp.b8[1] = x >> 8;
	_tmp.b8[0] = x & 0xff;
	return _tmp.b16;
}
#define libusb_le16_to_cpu libusb_cpu_to_le16

#define LIBUSB_ERROR_NOT_FOUND (-5)

#endif
