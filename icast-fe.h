// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * SMIT iCast DTMB/DVC driver
 *
 * Copyright (C) 2026 Xiaodong Ni <nxiaodong520@gmail.com>
 */

#ifndef ICAST_FE_H
#define ICAST_FE_H

struct icast_fe_config {
	u8 dtmb;
};

struct dvb_frontend *icast_fe_attach(struct dvb_usb_adapter *adap, const struct icast_fe_config *config);

#endif