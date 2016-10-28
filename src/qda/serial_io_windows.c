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

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "xmodem.h"

/* Support for up to COM 999 */
#define MAX_COM_PATH_LEN 11
#define COM_PATH_ESPACE_PREFIX "\\\\.\\"

static HANDLE serial_handle;
static DCB serial_initial_params;

void xmodem_putc(uint8_t *ch)
{
	WriteFile(serial_handle, ch, 1, NULL, NULL);
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
	DWORD n_bytes_read = 0;

	ReadFile(serial_handle, ch, 1, &n_bytes_read, NULL);
	switch (n_bytes_read) {
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
	COMMTIMEOUTS timeouts = {0};

	// Set COM port timeout settings
	timeouts.ReadIntervalTimeout = ms;
	timeouts.ReadTotalTimeoutConstant = ms;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = ms;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	if(SetCommTimeouts(serial_handle, &timeouts) == 0) {
		CloseHandle(serial_handle);
		return -1;
	}

	return 0;
}

int serial_io_open(char *path, int speed)
{
	DCB serial_params = {0};
	char escaped_path[MAX_COM_PATH_LEN];
	int escaping_path_ret_v;

	/*
         * MS naming convention for opening COM ports can vary for below 9 and
	 * above 9. However, the above 9 does also work for the below 9, so the
	 * COMXYZ port is escaped to match that pattern: \\.\COMXYZ
         */
	escaping_path_ret_v = snprintf (escaped_path, MAX_COM_PATH_LEN, "%s%s",
		COM_PATH_ESPACE_PREFIX, path);

 	/* Fail in case of error or input string too long */
	if ((escaping_path_ret_v < 0) ||
	   (escaping_path_ret_v >= MAX_COM_PATH_LEN)) {
		return -1;
	}

	/* Open the serial port */
	serial_handle = CreateFile(
		escaped_path, GENERIC_READ|GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (serial_handle == INVALID_HANDLE_VALUE) {
		return -1;
	}

	/* Save original serial params */
	if(GetCommState(serial_handle, &serial_initial_params) == 0) {
		CloseHandle(serial_handle);
		return -1;
	}

	/* Set the comm parameters */
	serial_params.ByteSize = 8;
	serial_params.StopBits = ONESTOPBIT;
	serial_params.Parity = NOPARITY;
	switch (speed) {
	case 1200:
		serial_params.BaudRate = CBR_1200;
		break;
	case 2400:
		serial_params.BaudRate = CBR_2400;
		break;
	case 4800:
		serial_params.BaudRate = CBR_4800;
		break;
	case 9600:
		serial_params.BaudRate = CBR_9600;
		break;
	case 19200:
		serial_params.BaudRate = CBR_19200;
		break;
	case 38400:
		serial_params.BaudRate = CBR_38400;
		break;
	case 57600:
		serial_params.BaudRate = CBR_57600;
		break;
	case 115200:
		serial_params.BaudRate = CBR_115200;
		break;
	default:
		errno = EINVAL;
		return -1;
	}
	if(SetCommState(serial_handle, &serial_params) == 0) {
		CloseHandle(serial_handle);
		return -1;
	}

	return 0;
}

int serial_detach(void)
{

	if (EscapeCommFunction(serial_handle, SETRTS) == 0) {
		return -1;
	}

	/* Keep RTS pulled low for 100 ms. */
	usleep(100000);


	if (EscapeCommFunction(serial_handle, CLRRTS) == 0) {
		return -1;
	}

	return 0;
}

int serial_io_close(void)
{
	if (serial_handle == INVALID_HANDLE_VALUE) {
		return -1;
	}

	/* Set initial system settings. */
	if (SetCommState(serial_handle, &serial_initial_params) == 0) {
		return -1;
	}

	if (CloseHandle(serial_handle) == 0) {
		return -1;
	}

	return 0;
}
