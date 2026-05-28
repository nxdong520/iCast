// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * SMIT iCast DTMB/DVC driver
 *
 * Copyright (C) 2026 Xiaodong Ni <nxiaodong520@gmail.com>
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0)
#include <dvb_frontend.h>
#else
#include <media/dvb_frontend.h>
#endif

#include "dvb-usb.h"
#include "icast-fe.h"
#include "smit.h"

#define CONFIG_DYNAMIC_DEBUG

#define USB_VID_SMIT							0x29DF
#define USB_PID_SMIT_ICAST				0x0001

DVB_DEFINE_MOD_OPT_ADAPTER_NR(adapter_nr);

static struct icast_fe_config icast_cfg = {
	.debug = 0,
};

static int icast_frontend_attach(struct dvb_usb_adapter *adap)
{
	if ((adap->fe_adap[0].fe = dvb_attach(icast_fe_attach, adap, &icast_cfg)) != NULL) {
		return 0;
	}
	info("not attached iCast usb device");
	return -ENODEV;
}

static struct usb_device_id icast_id_table[] = {
	{USB_DEVICE(USB_VID_SMIT, USB_PID_SMIT_ICAST)},
	{}
};

MODULE_DEVICE_TABLE(usb, icast_id_table);

static struct dvb_usb_device_properties icast_properties = {
	.num_adapters = 1,

	.adapter = {
		{
		.num_frontends = 1,
		.fe = {{
			.caps = DVB_USB_ADAP_HAS_PID_FILTER,
			.frontend_attach  = icast_frontend_attach,
			.stream = {
				.type = USB_BULK,
				.count = 8,
				.endpoint = SMIT_BULK_ENDPOINT_STREAM,
				.u = {
					.bulk = {
						.buffersize = 8192,
					}
				}
			},
		}},	
		},
	},
	.num_device_descs = 1,
	.devices = {
		{
			.name = "iCast DTMB/DVBC USB adapter",
		  .warm_ids = {&icast_id_table[0], NULL},
		},
	}
};

static int smit_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device *udev;
  int size;
  int ret;
  u8 *buf;

  udev = interface_to_usbdev(intf);

  buf = kmalloc(4, GFP_KERNEL);
  buf[0] = 0xFE;
  buf[1] = 0x00;
  buf[2] = 0x10;
  buf[3] = 0x00;
  
  ret = usb_bulk_msg(udev, usb_sndbulkpipe(udev, SMIT_BULK_ENDPOINT_CMD_SEND), buf, 4, &size, 500);
  if ( ret==0 )
  {
    ret = usb_bulk_msg(udev, usb_rcvbulkpipe(udev, SMIT_BULK_ENDPOINT_CMD_RECEIVE), buf, 4, &size, 500);
    if ( ret==0 )
    {
	    ret = smit_init(udev);
	    if ( ret==0 )
    	{
    		if (0 == dvb_usb_device_init(intf, &icast_properties, THIS_MODULE, 0LL, adapter_nr))
    		{
    			kfree(buf);
    			return 0;
    		}
    	}
    }
  }
  
  smit_reset(udev);
  kfree(buf);
  
  return -EINVAL;
}

static struct usb_driver smit_driver = {
	.name = KBUILD_MODNAME,
	.probe = smit_probe,
	.disconnect = dvb_usb_device_exit,
	.id_table = icast_id_table,
};

static int __init smit_module_init(void)
{
	int result = 0;
	if ((result = usb_register(&smit_driver))) {
		err("usb_register failed. Error number %d", result);
	}
	return result;
}

static void __exit smit_module_exit(void)
{
	usb_deregister(&smit_driver);
}

module_init(smit_module_init);
module_exit(smit_module_exit);

MODULE_DESCRIPTION("SMIT iCast driver");
MODULE_AUTHOR("Xiaodong Ni <nxiaodong520@gmail.com>");
MODULE_LICENSE("GPL");

