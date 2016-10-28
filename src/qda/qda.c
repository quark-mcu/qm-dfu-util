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

#include "qda_packets.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "qda.h"
#include "usb_dfu.h"

/* For x86 hosts we do not have to do any conversion */
#define htoq32(val) (val)
#define htoq16(val) (val)

#define qtoh32(val) (val)
#define qtoh16(val) (val)

#define DEBUG_MSG (0)

#if DEBUG_MSG
#define printd(...) printf(__VA_ARGS__)
#else
#define printd(...)
#endif

#define FAIL_IF(statement) do { \
		if(statement) { \
			printd("[FAIL]\n"); \
			return -1; \
		} \
	} while(0) \

qda_conf_t *qda_conf;

/* NOTE: make the size of the payload dynamic */
static uint8_t qda_buf[8192];

int qda_init(qda_conf_t *conf)
{
	qda_conf = conf;
	return 0;
}

int qda_reset()
{
	printd("qda_reset...\t");
	int rc;
	qda_pkt_t *req, *resp;

	req = (qda_pkt_t *)qda_buf;
	req->type = htoq32(QDA_PKT_RESET);

	rc = qda_conf->send((uint8_t *)req, sizeof(req->type));
	FAIL_IF(rc < 0);
	rc = qda_conf->receive(qda_buf, sizeof(qda_buf));
	FAIL_IF(rc >= (int) sizeof(qda_buf));

	resp = (qda_pkt_t *)qda_buf;
	FAIL_IF(resp->type != htoq32(QDA_PKT_ACK));
	printd("[DONE]\n");
	return 0;
}

int qda_get_dev_desc(qda_if_t *dif)
{
	printd("qda_get_dev_desc...\t");
	int rc;
	qda_pkt_t *req, *resp;
	dev_desc_resp_payload_t *pl;

	req = (qda_pkt_t *)qda_buf;
	req->type = htoq32(QDA_PKT_DEV_DESC_REQ);

	rc = qda_conf->send((uint8_t *)req, sizeof(req->type));
	FAIL_IF(rc < 0);
	qda_conf->receive(qda_buf, sizeof(qda_buf));

	resp = (qda_pkt_t *)qda_buf;
	FAIL_IF(resp->type != htoq32(QDA_PKT_DEV_DESC_RESP));
	pl = (dev_desc_resp_payload_t *)resp->payload;

	dif->product = qtoh16(pl->id_product);
	dif->vendor = qtoh16(pl->id_vendor);
	dif->bcdDevice = qtoh16(pl->bcd_device);

	printd("[DONE]\n");
	printd("\tvendId: 0x%04x\n", dif->vendor);
	printd("\tprodId: 0x%04x\n", dif->product);
	printd("\tbcd:    0x%04x\n", dif->bcdDevice);

	return 0;
}

int qda_get_dfu_desc(qda_if_t *dif)
{
	printd("qda_get_dfu_desc...\t");
	int rc;
	qda_pkt_t *req, *resp;
	dfu_desc_resp_payload_t *pl;

	req = (qda_pkt_t *)qda_buf;
	req->type = htoq32(QDA_PKT_DFU_DESC_REQ);

	rc = qda_conf->send((uint8_t *)req, sizeof(req->type));
	FAIL_IF(rc < 0);
	qda_conf->receive(qda_buf, sizeof(qda_buf));

	resp = (qda_pkt_t *)qda_buf;
	FAIL_IF(resp->type != htoq32(QDA_PKT_DFU_DESC_RESP));
	pl = (dfu_desc_resp_payload_t *)resp->payload;

	/* TODO: save alt setting number, attributes and timeout
	desc->num_alt_settings = pl->num_alt_settings;
	desc->bm_attributes = pl->bm_attributes;
	desc->detach_timeout = qtoh16(pl->detach_timeout);
	*/

	dif->func_dfu.wTransferSize = qtoh16(pl->transfer_size);
	dif->func_dfu.bcdDFUVersion = qtoh16(pl->bcd_dfu_ver);
	printd("\twTransferSize: %d\n", dif->func_dfu.wTransferSize);
	printd("\tbcdDFUversion: 0x%04x\n", dif->func_dfu.bcdDFUVersion);

	printd("[DONE]\n");
	return 0;
}

int qda_set_alt_setting(uint8_t alt)
{
	printd("qda_set_dfu_alt_setting...\t");
	int rc;
	qda_pkt_t *req, *resp;
	set_alt_setting_payload_t *pl;

	req = (qda_pkt_t *)qda_buf;
	req->type = htoq32(QDA_PKT_DFU_SET_ALT_SETTING);
	pl = (set_alt_setting_payload_t *)req->payload;
	pl->alt_setting = alt;

	rc = qda_conf->send((uint8_t *)req, sizeof(req->type) + sizeof(*pl));
	FAIL_IF(rc < 0);
	qda_conf->receive(qda_buf, sizeof(qda_buf));

	resp = (qda_pkt_t *)qda_buf;
	FAIL_IF(resp->type != htoq32(QDA_PKT_ACK));
	printd("[DONE]\n");
	return 0;
}

int qda_dfu_detach(void)
{
	printd("qda_dfu_detach...\t");
	FAIL_IF(qda_conf->detach() < 0);
	printd("[DONE]\n");
	return 0;
}

int qda_dfu_download(uint16_t len, uint16_t transaction, const uint8_t *data)
{
	printd("qda_dfu_dnload (len=%d)\nstarting...\t\t", len);
	int rc;
	qda_pkt_t *req, *resp;
	dnload_req_payload_t *pl;

	req = (qda_pkt_t *)qda_buf;
	req->type = htoq32(QDA_PKT_DFU_DNLOAD_REQ);
	pl = (dnload_req_payload_t *)req->payload;
	pl->data_len = len;
	pl->block_num = transaction;
	if (len > (sizeof(qda_buf) - sizeof(*req) - sizeof(*pl))) {
		return -1;
	}
	memcpy(pl->data, data, len);

	rc = qda_conf->send((uint8_t *)req,
				sizeof(*req) + sizeof(*pl) + pl->data_len);
	FAIL_IF(rc < 0);
	qda_conf->receive(qda_buf, sizeof(qda_buf));

	resp = (qda_pkt_t *)qda_buf;
	FAIL_IF(resp->type != htoq32(QDA_PKT_ACK));

	printd("[DONE]\n");
	return 0;
}

int qda_dfu_upload(uint16_t len, uint16_t transaction, uint8_t *data)
{
	printd("qda_dfu_upload...\t");
	int rc;
	qda_pkt_t *req, *resp;
	upload_req_payload_t *pl_req;
	upload_resp_payload_t *pl_resp;
	int retv;

	req = (qda_pkt_t *)qda_buf;
	req->type = htoq32(QDA_PKT_DFU_UPLOAD_REQ);
	pl_req = (upload_req_payload_t *)req->payload;
	pl_req->max_data_len = len;
	pl_req->block_num = transaction;

	rc = qda_conf->send((uint8_t *)req, sizeof(*req) + sizeof(*pl_req));
	FAIL_IF(rc < 0);
	qda_conf->receive(qda_buf, sizeof(qda_buf));

	resp = (qda_pkt_t *)qda_buf;
	FAIL_IF(resp->type != htoq32(QDA_PKT_DFU_UPLOAD_RESP));
	pl_resp = (upload_resp_payload_t *)resp->payload;
	retv = qtoh16(pl_resp->data_len);
	FAIL_IF(retv > len);
	memcpy(data, pl_resp->data, retv);

	printd("[DONE]\n");
	printd("\tlen: %d\t", retv);
	return retv;
}

int qda_dfu_getstatus(dfu_status_t *status)
{
	printd("qda_dfu_getstatus...\t");
	int rc;
	qda_pkt_t *req, *resp;
	get_status_resp_payload_t *pl;
	req = (qda_pkt_t *)qda_buf;
	req->type = htoq32(QDA_PKT_DFU_GETSTATUS_REQ);

	rc = qda_conf->send((uint8_t *)req, sizeof(*req));
	FAIL_IF(rc < 0);
	qda_conf->receive(qda_buf, sizeof(qda_buf));

	resp = (qda_pkt_t *)qda_buf;
	FAIL_IF(resp->type != htoq32(QDA_PKT_DFU_GETSTATUS_RESP));
	pl = (get_status_resp_payload_t *)resp->payload;
	status->bState = pl->state;
	status->bStatus = pl->status;
	status->bwPollTimeout = qtoh32(pl->poll_timeout);

	printd("[DONE]\n");
	return 0;
}

int qda_dfu_clrstatus()
{
	printd("qda_dfu_clrstatus...\t");
	int rc;
	qda_pkt_t *req, *resp;
	req = (qda_pkt_t *)qda_buf;
	req->type = htoq32(QDA_PKT_DFU_CLRSTATUS);

	rc = qda_conf->send((uint8_t *)req, sizeof(*req));
	FAIL_IF(rc < 0);
	qda_conf->receive(qda_buf, sizeof(qda_buf));

	resp = (qda_pkt_t *)qda_buf;
	FAIL_IF(resp->type != htoq32(QDA_PKT_ACK));

	printd("[DONE]\n");
	return 0;
}

int qda_dfu_getstate()
{
	printd("qda_dfu_getstate...\t");
	int rc;
	qda_pkt_t *req, *resp;
	get_state_resp_payload_t *pl;
	req = (qda_pkt_t *)qda_buf;
	req->type = htoq32(QDA_PKT_DFU_GETSTATE_REQ);

	rc = qda_conf->send((uint8_t *)req, sizeof(*req));
	FAIL_IF(rc < 0);
	qda_conf->receive(qda_buf, sizeof(qda_buf));

	resp = (qda_pkt_t *)qda_buf;
	FAIL_IF(resp->type != htoq32(QDA_PKT_DFU_GETSTATE_RESP));
	pl = (get_state_resp_payload_t *)resp->payload;

	printd("[DONE] (%s)\n", dfu_state_to_string(pl->state));
	return pl->state;
}

int qda_dfu_abort()
{
	printd("qda_dfu_abort...\t\t");
	int rc;
	qda_pkt_t *req, *resp;
	req = (qda_pkt_t *)qda_buf;
	req->type = htoq32(QDA_PKT_DFU_ABORT);

	rc = qda_conf->send((uint8_t *)req, sizeof(*req));
	FAIL_IF(rc < 0);
	qda_conf->receive(qda_buf, sizeof(qda_buf));

	resp = (qda_pkt_t *)qda_buf;
	FAIL_IF(resp->type != htoq32(QDA_PKT_ACK));

	printd("[DONE]\n");
	return 0;
}

/* The following dfu_status functions are copied from dfu.c */
/* TODO: consider reusing the existing functions from dfu.c */

/* Chapter 6.1.2 */
static const char *dfu_status_names[] = {
	/* DFU_STATUS_OK */
	"No error condition is present",
	/* DFU_STATUS_errTARGET */
	"File is not targeted for use by this device",
	/* DFU_STATUS_errFILE */
	"File is for this device but fails some vendor-specific test",
	/* DFU_STATUS_errWRITE */
	"Device is unable to write memory",
	/* DFU_STATUS_errERASE */
	"Memory erase function failed",
	/* DFU_STATUS_errCHECK_ERASED */
	"Memory erase check failed",
	/* DFU_STATUS_errPROG */
	"Program memory function failed",
	/* DFU_STATUS_errVERIFY */
	"Programmed memory failed verification",
	/* DFU_STATUS_errADDRESS */
	"Cannot program memory due to received address that is out of range",
	/* DFU_STATUS_errNOTDONE */
	"Received DFU_DNLOAD with wLength = 0, but device does not think that it "
	"has all data yet",
	/* DFU_STATUS_errFIRMWARE */
	"Device's firmware is corrupt. It cannot return to run-time (non-DFU) "
	"operations",
	/* DFU_STATUS_errVENDOR */
	"iString indicates a vendor specific error",
	/* DFU_STATUS_errUSBR */
	"Device detected unexpected USB reset signalling",
	/* DFU_STATUS_errPOR */
	"Device detected unexpected power on reset",
	/* DFU_STATUS_errUNKNOWN */
	"Something went wrong, but the device does not know what it was",
	/* DFU_STATUS_errSTALLEDPKT */
	"Device stalled an unexpected request"};

static const char *dfu_state_names[] = {
	/* DFU_STATE_appIDLE */
	"appIDLE",
	/* DFU_STATE_appDETACH */
	"appDETACH",
	/* DFU_STATE_dfuIDLE */
	"dfuIDLE",
	/* DFU_STATE_dfuDNLOAD_SYNC */
	"dfuDNLOAD-SYNC",
	/* DFU_STATE_dfuDNBUSY */
	"dfuDNBUSY",
	/* DFU_STATE_dfuDNLOAD_IDLE */
	"dfuDNLOAD-IDLE",
	/* DFU_STATE_dfuMANIFEST_SYNC */
	"dfuMANIFEST-SYNC",
	/* DFU_STATE_dfuMANIFEST */
	"dfuMANIFEST",
	/* DFU_STATE_dfuMANIFEST_WAIT_RST */
	"dfuMANIFEST-WAIT-RESET",
	/* DFU_STATE_dfuUPLOAD_IDLE */
	"dfuUPLOAD-IDLE",
	/* DFU_STATE_dfuERROR */
	"dfuERROR",
};

const char *qda_dfu_state_to_string(int state)
{
	if (state > DFU_STATE_dfuERROR)
		return "INVALID STATE ID";
	return dfu_state_names[state];
}

const char *qda_dfu_status_to_string(int status)
{
	if (status > DFU_STATUS_errSTALLEDPKT)
		return "INVALID";
	return dfu_status_names[status];
}
