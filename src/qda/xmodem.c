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
#include <stdint.h>
#include <string.h>

#include "xmodem.h"

/* The maximum number of consecutive RX errors XMODEM tolerates */
#define MAX_RX_ERRORS (5)

/* Timeouts in milliseconds */
#define TIMEOUT_STD 3000
#define TIMEOUT_ERR 300

/* Custom value, not transfered via XMODEM, but used as return codes */
#define ERR (0xFF)
#define DUP (0xFE)

/* XMODEM control bytes */
#define SOH (0x01)
#define EOT (0x04)
#define ACK (0x06)
#define NAK (0x15)
#define CAN (0x18)

/* XMODEM block size */
#define PACKET_PAYLOAD_SIZE (XMODEM_BLOCK_SIZE)

/* CRC-16 CCITT */
#define POLY 0x1021

/* Activate debug messages by defining DEBUG_MSG to 1 */
#define DEBUG_MSG (0)

#if DEBUG_MSG
#define printd(...) printf(__VA_ARGS__)
#else
#define printd(...)
#endif

extern int xmodem_getc(uint8_t *ch);
extern void xmodem_putc(uint8_t *ch);
extern int xmodem_set_timeout(int ms);

/**
 * The XMODEM packet buffer.
 *
 * This buffer is used for both incoming and outgoing packets.
 */
static struct __attribute__((__packed__)) xmodem_packet {
	uint8_t soh;
	uint8_t seq_no;
	uint8_t seq_no_inv;
	uint8_t data[PACKET_PAYLOAD_SIZE];
	uint8_t crc_u8[2];
} pkt_buf;

/**
 *  Compute CRC.
 *
 *  This function computes CRC 16bits CCITT of the payload, which means:
 *  - with CRC initialized to 0xffff
 *  - with 16 'zero' bits appended to the end of the message
 *  - 0x1021 for the polynomial
 *
 *  @retval computed CRC value
 */
static uint16_t crc_xmodem(void)
{
	uint8_t i, b;
	uint8_t data;
	uint32_t crc = 0;

	for (i = 0; i < PACKET_PAYLOAD_SIZE; i++) {
		data = pkt_buf.data[i];
		for (b = 0; b < 8; b++) {
			crc <<= 1;
			/* add MSB bit of data to message */
			if (data & 0x80) {
				crc |= 1;
			}
			if (crc & 0x10000)
				crc ^= POLY;
			data <<= 1;
		}
	};

	/* append 0 */
	for (i = 0; i < 16; i++) {
		if (crc & 0x8000)
			crc = (crc << 1) ^ POLY;
		else
			crc <<= 1;
	}

	return (uint16_t)(crc);
}

/**
 * Send a single XMODEM packet.
 *
 * @param[in] data     The payload of the packet.
 * @param[in] data_len The length of the payload. Must be at most 128 bytes.
 *		       If less, (random) padding is automatically added.
 * @param[in] pkt_no   The desired packet sequence number.
 *
 * @return Resulting status code.
 * @retval 0 Success (only possible retval for now).
 */
static int xmodem_send_pkt(const uint8_t *data, size_t data_len, uint8_t pkt_no)
{
	size_t i;
	uint8_t *buf;
	uint16_t crc;

	printd("xmodem_send_pkt(): pkt_no: %d\n", pkt_no);
	pkt_buf.soh = SOH;
	memcpy(pkt_buf.data, data, data_len);
	crc = crc_xmodem();
	pkt_buf.crc_u8[0] = (crc >> 8) & 0xFF;
	pkt_buf.crc_u8[1] = crc & 0xFF;
	pkt_buf.seq_no = pkt_no;
	pkt_buf.seq_no_inv = ~pkt_no;
	buf = (uint8_t *)&pkt_buf;
	/* Send the packet */
	for (i = 0; i < sizeof(pkt_buf); i++) {
		xmodem_putc(&buf[i]);
	}

	return 0;
}

/**
 * Try to send an XMODEM packet for MAX_RETRANSMIT times.
 *
 * This function sends an XMODEM packet and checks if an ACK is received. If no
 * ACK is received, the packet is retransmitted. This is done until
 * 'MAX_RETRANSMIT' is exceeded.
 *
 * @param[in] data     The payload of the packet.
 * @param[in] data_len The length of the payload. Must be at most 128 bytes.
 *		       If less, (random) padding is automatically added.
 * @param[in] pkt_no   The packet sequence number.
 *
 * @return Exit status.
 * @retval 0  Success, the packet has been transmitted and an ACK received.
 * @retval -1 Error, retransmit count exceeded.
 */
static int xmodem_send_pkt_with_retry(const uint8_t *data, size_t data_len,
									  uint8_t pkt_no)
{
	uint8_t retransmit = MAX_RETRANSMIT;
	uint8_t rsp;

	printd("xmodem_send_pkt_with_retry(): pkt_no: %d\n", pkt_no);
	while (retransmit--) {
		xmodem_send_pkt(data, data_len, pkt_no);
		rsp = ERR;
		xmodem_getc(&rsp);
		if (rsp == ACK) {
			printd("xmodem_send_pkt_with_retry(): done\n");
			return 0;
		}
		printd("xmodem_send_pkt_with_retry(): failure (%d) 0x%02x\n",
			   retransmit, rsp);
	}

	return -1;
}

/**
 * Try to send a byte for MAX_RETRANSMIT times.
 *
 * This function sends a byte (typically an XMODEM control byte) and checks if
 * an ACK is received. If no ACK is received, the byte is retransmitted. This is
 * done until 'MAX_RETRANSMIT' is exceeded.
 *
 * @param[in] cmd The byte to send.
 *
 * @return Exit status.
 * @retval 0  Success, the byte has been transmitted and an ACK received.
 * @retval -1 Error, retransmit count exceeded.
 */
static int xmodem_send_byte_with_retry(uint8_t cmd)
{
	uint8_t retransmit = MAX_RETRANSMIT;
	uint8_t rsp;

	while (retransmit--) {
		xmodem_putc(&cmd);
		rsp = ERR;
		xmodem_getc(&rsp);
		if (rsp == ACK) {
			return 0;
		}
		printd("xmodem_send_pkt_with_retry(): failure (%d) 0x%02x\n",
			   retransmit, rsp);
	}

	return -1;
}

/*
 * Receive an XMODEM packet.
 *
 * @param[in] exp_seq_no The expected sequence number of the packet to be
 * 			 received.
 * @param[in] data       The buffer where to store the packet payload.
 * @param[in] len        The size of the buffer.
 *
 * @return Status code.
 * @retval SOH The packet has been successful received.
 * @retval DUP The received packet is a duplicate of the previous one (based on
 *             the expected sequence number); nothing has not been written to
 *             the data buffer.
 * @retval ERR An error has occurred (either a timeout or the reception of
 * 	       invalid / corrupted data), but the XMODEM session is not
 *	       compromised.
 * @retval CAN An unrecoverable error has occurred (either the sender and
 *	       receiver have lost sync or the passed buffer is too small).
 * @retval EOT The sender notified the end of transmission (i.e., there are no
 *	       more packets to receive).
 */
static int xmodem_read_pkt(uint8_t exp_seq_no, uint8_t *data, size_t len)
{
	uint8_t cmd;
	uint16_t crc_recv; /* received CRC */
	uint16_t crc_comp; /* computed CRC */
	uint8_t *buf;
	uint8_t *buf_end;

	cmd = ERR;

	if (xmodem_getc(&cmd) < 0) {
		return ERR;
	}

	switch (cmd) {
	case SOH:
		printd("xmodem_read_pkt(): cmd: SOH\n");
		break;
	case EOT:
		printd("xmodem_read_pkt(): cmd: EOT\n");
		return EOT;
	default:
		/*
		 * Unexpected cmd case.
		 *
		 * This includes the case of a corrupted/lost SOH; therefore,
		 * we should check if other bytes are arriving and, in case,
		 * discard them before returning (and replying with a NAK).
		 *
		 * This is the purpose of the following loop.
		 */
		printd("xmodem_read_pkt(): cmd: unexpected ctrl byte (0x%x)\n", cmd);
		/* Wait until the sender stops sending bytes. We change timeout value
		 * for the next loop */
		xmodem_set_timeout(TIMEOUT_ERR);
		while (xmodem_getc(&cmd) >= 0)
			;
		xmodem_set_timeout(TIMEOUT_STD);
		return ERR;
	}

	/* Read the rest of the packet (seq_no, ~seq_no, data, and CRC) */
	/* Start from seq_no, since we have already read SOH */
	buf = (uint8_t *)&pkt_buf.seq_no;
	/* Compute end of buffer */
	buf_end = (uint8_t *)(&pkt_buf + 1);
	while (buf < buf_end) {
		if (xmodem_getc(buf++) < 0) {
			printd("xmodem_read_pkt(): pkt: ERROR: timeout\n");
			printd("----\n");
			/* This is a timeout error */
			return ERR;
		}
	}

	/* Check sequence number fields and CRC */
	crc_comp = crc_xmodem();
	crc_recv = (pkt_buf.crc_u8[0] << 8) | pkt_buf.crc_u8[1];
	/*
	 * NOTE: Using 'a == (~a &FF)' instead of 'a == ~a', since the latter
	 * leads to a compilation error due to the following GCC bug:
	 * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
	 */
	if ((pkt_buf.seq_no != (~pkt_buf.seq_no_inv & 0xFF)) ||
		(crc_recv != crc_comp)) {
		printd("xmodem_read_pkt(): pkt: ERROR: corrupted packet\n");
		return ERR;
	}
	/* Check packet numbers. */
	if ((pkt_buf.seq_no == (exp_seq_no - 1))) {
		printd("xmodem_read_pkt(): pkt: WARNING duplicated packet\n");
		return DUP;
	}
	if (pkt_buf.seq_no != exp_seq_no) {
		printd("xmodem_read_pkt(): pkt: ERROR: wrong seq number\n");
		return CAN;
	}

	/*
	 * If we reach this point, the packet is the expected one and it has
	 * been correctly received: now we can check that the buffer is big
	 * enough to hold the payload (NOTE: this check should not be
	 * anticipated, otherwise we risk to return a CAN in case of a simple
	 * EOT from the sender).
	 */
	if (len < sizeof(pkt_buf.data)) {
		printd("xmodem_read_pkt(): pkt: "
			   "ERROR: user buffer out of space\n");
		return CAN;
	}
	memcpy(data, pkt_buf.data, sizeof(pkt_buf.data));
	printd("xmodem_read_pkt(): pkt: received correctly\n");

	return SOH;
}

int xmodem_receive_package(uint8_t *buf, size_t buf_len)
{
	int status;
	uint8_t exp_seq_no;
	uint8_t nak;
	uint8_t cmd;
	int retv;
	int data_cnt;
	int err_cnt;

	xmodem_set_timeout(TIMEOUT_STD);

	/* XMODEM sequence number starts from 1 */
	exp_seq_no = 1;
	/* Reception is started by sending a 'C' */
	cmd = 'C';
	/*
	 * Until the first packet is received (i.e., the XMODEM transfer is
	 * started), we must nak with a 'C' instead of a regular NAK
	 * [this is an XMODEM-CRC peculiarity]
	 */
	nak = 'C';

	err_cnt = 0;
	data_cnt = 0;
	retv = -1;
	while (err_cnt < MAX_RX_ERRORS) {
		printd("xmodem_receive(): sending cmd: %x\n", cmd);
		xmodem_putc(&cmd);
		status = xmodem_read_pkt(exp_seq_no, &buf[data_cnt], buf_len);
		switch (status) {
		case SOH:
			nak = NAK;
			data_cnt += sizeof(pkt_buf.data);
			buf_len -= sizeof(pkt_buf.data);
			exp_seq_no++;
			err_cnt = 0;
		/* no 'break' on purpose */
		case DUP:
			/*
			 * We must acknowledge duplicated packets to have the
			 * sender transmit the next packet
			 */
			cmd = ACK;
			break;
		case EOT:
			cmd = ACK;
			retv = data_cnt;
			goto exit;
		case CAN:
			cmd = CAN;
			goto exit;
		default:
			err_cnt++;
			cmd = nak;
		}
	}
exit:
	if (retv < 0) {
		printd("xmodem_receive(): ERROR: reception failed\n");
	}
	xmodem_putc(&cmd);

	return retv;
}

int xmodem_transmit_package(uint8_t *data, size_t len)
{
	int mlen;
	uint8_t retransmit;
	uint8_t rsp;
	uint8_t pkt_no;

	xmodem_set_timeout(TIMEOUT_STD);
	retransmit = MAX_RETRANSMIT;

	while (retransmit--) {
		printd("xmodem_transmit(): waiting for 'C' (%d)\n", retransmit);
		rsp = ERR;
		xmodem_getc(&rsp);
		if (rsp == 'C') {
			goto start_transmit;
		}
	}

	return -1;

start_transmit:
	printd("xmodem_transmit(): starting transmission\n");
	pkt_no = 1;
	/* Send packets as long data */
	while (len) {
		mlen = (len >= sizeof(pkt_buf.data)) ? sizeof(pkt_buf.data) : len;
		if (xmodem_send_pkt_with_retry(data, mlen, pkt_no) < 0) {
			return -1;
		}
		data += mlen;
		len -= mlen;
		pkt_no++;
	}
	if (xmodem_send_byte_with_retry(EOT) < 0) {
		return -1;
	}

	return (pkt_no - 1) * 128;
}
