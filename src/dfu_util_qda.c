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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "qda.h"
#include "xmodem.h"

#define DEBUG_MSG (0)

#if DEBUG_MSG
#define printd(...) printf(__VA_ARGS__)
#else
#define printd(...)
#endif

/* called from QDA */
int dfu_util_qda_send(uint8_t *data, size_t len)
{
	printd("QDA send:    (%d)\n", len);
	return xmodem_transmit_package(data, len);
}

/* called from QDA */
size_t dfu_util_qda_receive(uint8_t *data, size_t len)
{
	int ret;

	printd("QDA receive: (%d)\n", len);
	ret = xmodem_receive_package(data, len);

	return (ret==-1)?0:ret;
}

