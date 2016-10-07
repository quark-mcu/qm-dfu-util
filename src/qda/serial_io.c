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
#include <termio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "xmodem.h"

static int serial_handle;
static struct termios tio_initial;

static void _signal_handler(int sig);

void xmodem_putc(uint8_t *ch)
{
	size_t size = write(serial_handle, ch, 1);
	(void)size;
}

/*
 * Get a character from the XMODEM I/O layer.
 *
 * This function must be blocking, but is also expected to timeout after a
 * certain amount of time. Moreover, in case of error, the function must not
 * set the output parameter (i.e., the pointed variable must remain unchanged).
 *
 * @param[out] ch A pointer to the variable where to store the read character.
 * 		  In case of error, the current value of the pointed variable
 * 		  is not modified.
 *
 * @return 0 on success, negative error code otherwise.
 * @retval -ETIMEDOUT in case of timeout.
 * @retval -EIO   in case of I/O error.
 */
int xmodem_getc(uint8_t *ch)
{
	ssize_t retv;

	retv = read(serial_handle, ch, 1);

	switch (retv) {
		case 1:
			/* We read one character as expected: success */
			return 0;
		case 0:
			/* We read 0 characters: we timed out */
			return -ETIMEDOUT;
		default:
			/* Error or unexpected value: generic I/O error */
			return -EIO;

	}
}

int xmodem_set_timeout(int ms)
{
	struct termios tio;
	if(tcgetattr(serial_handle, &tio) < 0)
	   return -1;

	tio.c_cc[VTIME] = ms / 100;
	return tcsetattr(serial_handle, TCSANOW, &tio);
}

int serial_io_open(char *path, int speed)
{
	struct termios tio;
	memset(&tio, 0, sizeof(tio));
	tio.c_iflag = 0;
	tio.c_oflag = 0;
	/* 8n1, see termios.h for more information */
	tio.c_cflag = CS8 | CREAD | CLOCAL;
	tio.c_lflag = 0;
	tio.c_cc[VMIN] = 0;
	/* Set 3s as a standart value. Will be set by xmodem_set_timeout before
	 * each run. */
	tio.c_cc[VTIME] = 30;

	serial_handle = open(path, O_RDWR | O_NOCTTY);

	/* Check if file is open */
	if (serial_handle == -1) {
		return -1;
	}

	/* Check if file is a terminal */
	if (isatty(serial_handle) != 1) {
		return -1;
	}

	/* Save initial system settings */
	if(tcgetattr(serial_handle, &tio_initial)) {
		return -1;
	}

	/* Set signal handler for SIGINT to catch user initiated ^C signals. This
	 * allows serial_io to reset the serial settings and close the serial port
	 * before the program exits. This is done after the port is opened and
	 * before new configuration is set. */
	signal(SIGINT, _signal_handler);

	speed_t serial_speed;
	switch (speed) {
	case 1200:
		serial_speed = B1200;
		break;
	case 2400:
		serial_speed = B2400;
		break;
	case 4800:
		serial_speed = B4800;
		break;
	case 9600:
		serial_speed = B9600;
		break;
	case 19200:
		serial_speed = B19200;
		break;
	case 38400:
		serial_speed = B38400;
		break;
	case 57600:
		serial_speed = B57600;
		break;
	case 115200:
		serial_speed = B115200;
		break;
	default:
		errno = EINVAL;
		return -1;
	}
	cfsetospeed(&tio, serial_speed);
	cfsetispeed(&tio, serial_speed);

	if (tcsetattr(serial_handle, TCSANOW, &tio) < 0) {
		return -1;
	}
	return serial_handle;
}

int serial_detach(void)
{
	int status = 0;
	int ret = 0;
	ret = ioctl(serial_handle, TIOCMGET, &status);

	if (ret < 0) {
		return ret;
	}

	status |= TIOCM_RTS;
	ret = ioctl(serial_handle, TIOCMSET, &status);

	if (ret < 0) {
		return ret;
	}
	/* Keep RTS pulled low for 100 ms. */
	usleep(100000);
	status &= ~TIOCM_RTS;
	ret = ioctl(serial_handle, TIOCMSET, &status);
	if (ret < 0) {
		return ret;
	}

	return ret;
}

int serial_io_close(void)
{
	if (serial_handle == -1) {
		return -1;
	}

	/* Set initial system settings. */
	if(tcsetattr(serial_handle, TCSANOW, &tio_initial)) {
		return -1;
	}

	return close(serial_handle);
}

#if _BullseyeCoverage
#pragma BullseyeCoverage off
#endif

static void _signal_handler(int sig)
{
	/* Clean up open serial port before exiting. */
	serial_io_close();

	/* Exit codes for kill signals are (128 + signal_number). */
	exit(128 + sig);
}

#if _BullseyeCoverage
#pragma BullseyeCoverage on
#endif
