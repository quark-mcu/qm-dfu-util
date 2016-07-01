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

#ifndef __QDA_PACKETS_H__
#define __QDA_PACKETS_H__

#include <stdint.h>

#define __ATTR_PACKED__ __attribute__((__packed__))

/**
 * QDA packet types
 */
typedef enum {
	/* Host requests */
	QDA_PKT_RESET = 0x4D550000,
	QDA_PKT_DEV_DESC_REQ = 0x4D550005,
	QDA_PKT_DFU_DESC_REQ = 0x4D5501FF,
	QDA_PKT_DFU_SET_ALT_SETTING = 0x4D5501FE,
	QDA_PKT_DFU_DETACH = 0x4D550100,
	QDA_PKT_DFU_DNLOAD_REQ = 0x4D550101,
	QDA_PKT_DFU_UPLOAD_REQ = 0x4D550102,
	QDA_PKT_DFU_GETSTATUS_REQ = 0x4D550103,
	QDA_PKT_DFU_CLRSTATUS = 0x4D550104,
	QDA_PKT_DFU_GETSTATE_REQ = 0x4D550105,
	QDA_PKT_DFU_ABORT = 0x4D550106,
	/* Device responses */
	QDA_PKT_ATTACH = 0x4D558001,
	QDA_PKT_DETACH = 0x4D558002,
	QDA_PKT_ACK = 0x4D558003,
	QDA_PKT_STALL = 0x4D558004,
	QDA_PKT_DEV_DESC_RESP = 0x4D558005,
	QDA_PKT_DFU_DESC_RESP = 0x4D5581FF,
	QDA_PKT_DFU_UPLOAD_RESP = 0x4D558102,
	QDA_PKT_DFU_GETSTATUS_RESP = 0x4D558103,
	QDA_PKT_DFU_GETSTATE_RESP = 0x4D558105,

} qda_pkt_type_t;

/**
 * Generic QDA Packet structure
 */
typedef struct __ATTR_PACKED__ {
	uint32_t type;
	uint8_t payload[];
} qda_pkt_t;

/**
 * QDA_DNLOAD_REQ payload structure
 */
typedef struct __ATTR_PACKED__ {
	uint16_t data_len;
	uint16_t block_num;
	uint8_t data[];
} dnload_req_payload_t;

/**
 * QDA_UPLOAD_REQ payload structure
 */
typedef struct __ATTR_PACKED__ {
	uint16_t max_data_len;
	uint16_t block_num;
} upload_req_payload_t;

/**
 * QDA_USB_SET_ALT_SETTING payload structure
 */
typedef struct __ATTR_PACKED__ {
	uint8_t alt_setting;
} set_alt_setting_payload_t;

/**
 * QDA_UPLOAD_RESP payload structure
 */
typedef struct __ATTR_PACKED__ {
	uint16_t data_len;
	uint8_t data[];
} upload_resp_payload_t;

/**
 * QDA_USB_DEV_DESC_RESP payload structure
 */
typedef struct __ATTR_PACKED__ {
	uint16_t id_vendor;
	uint16_t id_product;
	uint16_t bcd_device;
} dev_desc_resp_payload_t;

/**
 * QDA_DFU_DESC_RESP payload structure
 */
typedef struct __ATTR_PACKED__ {
	uint8_t num_alt_settings;
	uint8_t bm_attributes;
	uint16_t detach_timeout;
	uint16_t transfer_size;
	uint16_t bcd_dfu_ver;
} dfu_desc_resp_payload_t;

/**
 * QDA_GET_STATUS_RESP payload structure
 */
typedef struct __ATTR_PACKED__ {
	/*
	 * As per the DFU spec, poll_timeout is 3-byte integer, but QDA stores
	 * it in a 4-byte field for simplicity
	 */
	uint32_t poll_timeout;
	uint8_t status;
	uint8_t state;
} get_status_resp_payload_t;

/**
 * QDA_GET_STATE_RESP payload structure
 */
typedef struct __ATTR_PACKED__ {
	uint8_t state;
} get_state_resp_payload_t;

#endif /* __QDA_PACKETS_H__ */
