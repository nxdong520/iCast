// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * SMIT iCast DTMB/DVC driver
 *
 * Copyright (C) 2026 Xiaodong Ni <nxiaodong520@gmail.com>
 */

#ifndef SMIT_H
#define SMIT_H

#define SMIT_BULK_ENDPOINT_CMD_SEND				0x01
#define SMIT_BULK_ENDPOINT_CMD_RECEIVE		0x82
#define SMIT_BULK_ENDPOINT_STREAM					0x84

int smit_reset(struct usb_device *udev);
int smit_set(struct usb_device *udev, u8 req, u8 *data, u8 len, int need_complete_flag);
int smit_tuner_stat(struct usb_device *udev, u8 *out, int checkcount);
int smit_init(struct usb_device *udev, u8 *dtmb);

#endif