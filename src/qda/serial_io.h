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

#ifndef _SERIAL_IO_H_
#define _SERIAL_IO_H_

#include <stdint.h>
#include "xmodem.h"

/**
 * Open serial port for XMODEM usage.
 *
 * @param[in] path Path to serial interface.
 * @param[in] speed XMODEM speed.
 * @param[out] handle Stores the resulting device handle.
 *
 * @retval Serial interface handle or error status
 * @retval >0 Serial interface handle.
 * @retval -1 Error (Check errno)
 */
int serial_io_open(const char *path, int speed);

/**
 * Close serial port after XMODEM usage.
 *
 * @retval Error Status
 * @retval 0 Success
 * @retval -1 Error (Check errno)
 */
int serial_io_close(void);


/**
 * Uses RTS line to simulate a DFU detach command.
 *
 * @retval Error Status
 * @retval 0 Success
 * @retval -1 Error (Check errno)
 */
int serial_detach(void);

#endif /* _SERIAL_IO_H_ */
