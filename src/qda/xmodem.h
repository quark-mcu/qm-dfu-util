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

#ifndef __XMODEM_H__
#define __XMODEM_H__

#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

/** XMODEM block size */
#define XMODEM_BLOCK_SIZE (128)

/**
 * @defgroup groupXMODEM XMODEM
 * @{
 */

/**
 * Switch XMODEM to receive mode.
 *
 * XMODEM starts to send 'C' (NAK-CRC) messages to the sender and waits for
 * incoming transmissions. Received data is copied into the provided buffer.
 *
 * This function is blocking and it does not timeout.
 *
 * @param[out] buf      Buffer where to store the received data.
 * @param[in]  buf_size The size of the buffer.
 *
 * @return Number of received bytes or negative error code. Note that XMODEM
 *         may add up to 127 padding bytes at the end of the real data.
 * @retval >0 Number of received bytes (including padding).
 * @retval -1 Error (either the reception failed for an unrecoverable protocol
 * 	      error or the provided buffer is too small)
 */
int xmodem_receive_package(uint8_t *buf, size_t buf_size);

/**
 * Switch XMODEM to transmit mode.
 *
 * XMODEM waits for 'C' (NAK-CRC) messages until the transmission begins.  The
 * package content is sent in 128 bytes frames. Extra (padding) data is added
 * to the last frame if the data size is not multiple of 128 bytes.
 *
 * This function is blocking, but may timeout.
 *
 * @param[in] data The data to send.
 * @param[in] len  The length of the data.
 *
 * @return Number of bytes actually transmitted (including padding) on success,
 * 	   negative error code otherwise.
 * @retval >0 Number of sent bytes (including padding).
 * @retval -1 Error (timeout or number of retries exceeded).
 */
int xmodem_transmit_package(uint8_t *data, size_t len);

/**
 * @}
 */

#endif
