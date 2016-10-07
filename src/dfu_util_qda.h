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

#ifndef DFU_UTIL_QDA_H
#define DFU_UTIL_QDA_H

#include <stdint.h>

enum mode {
	MODE_NONE,
	MODE_VERSION,
	MODE_LIST,
	MODE_DETACH,
	MODE_UPLOAD,
	MODE_DOWNLOAD
};

/**
 * Send a QDA message using XMODEM.
 *
 * This function is called by QDA as a send callback.
 *
 * @param[in] data Data to be sent.
 * @param[in] len Length of data to be sent.
 *
 * @return Error Status
 * @retval 0 Success
 * @retval <0 Error (Forwarded error code from XMODEM)
 */
int dfu_util_qda_send(uint8_t *data, size_t len);

/**
 * Receive a QDA message using XMODEM.
 *
 * This function is called by QDA as a receive callback.
 *
 * @param[out] data Data buffer for received bytes.
 * @param[in] len Maximum allocated bytes that can be received.
 *
 * @return Length of received data.
 */
size_t dfu_util_qda_receive(uint8_t *data, size_t len);

#endif /* DFU_UTIL_QDA_H */
