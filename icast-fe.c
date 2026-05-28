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
#include "smit.h"
#include "icast-fe.h"

#define CONFIG_DYNAMIC_DEBUG

struct icast_state {
	struct dvb_usb_device *dev;
  struct dvb_frontend frontend;
  u32 frequency;
  u32 symbol_rate;
  enum fe_modulation	modulation;
  u8 lock;
  u8 strength;
  u16 snr;
};

static int icast_fe_init(struct dvb_frontend *fe)
{
	struct icast_state *priv = fe->demodulator_priv;
	
	priv->frequency = 0;
	priv->symbol_rate = 0;

	return 0;
}

static void icast_fe_release(struct dvb_frontend *fe)
{
	struct icast_state *state = fe->demodulator_priv;

	kfree(state);
}

static int icast_fe_get_frontend(struct dvb_frontend *fe, struct dtv_frontend_properties *props)
{
  struct icast_state *priv = fe->demodulator_priv;

  fe->dtv_property_cache.frequency = priv->frequency;
  fe->dtv_property_cache.symbol_rate = priv->symbol_rate;

  return 0;
}

#define DWORD_TO_BYTES(dword, bytes, position) \
  bytes[position] = (u8)(dword & 0xFF); \
  bytes[position+1] = (u8)((dword>>8) & 0xFF); \
  bytes[position+2] = (u8)((dword>>16) & 0xFF); \
  bytes[position+3] = (u8)((dword>>24) & 0xFF);

static int icast_fe_set_frontend(struct dvb_frontend *fe)
{
  u8 cmd9[32] = {
  	0x01, 0x90, 0x02, 0x00, 0x03, 0x9F, 0x9A, 0x07,
  	0x17,	0x00, 0x00, 0x14, 0x00, 0x03, 0x00, 0x10,
  	0xB8,	0xF3, 0x03, 0x00, 0xDB, 0x1A, 0x00, 0x00,
  	0x40,	0x00, 0x00, 0x00, 0x08, 0x01, 0x00, 0x00
  }; 
  u8 buf[900];
  u32 frequency, symbol_rate;
  struct icast_state *priv = fe->demodulator_priv;

	frequency = (u32)(fe->dtv_property_cache.frequency / 1000);
  DWORD_TO_BYTES(frequency, cmd9, 16);
  
  if ( fe->dtv_property_cache.delivery_system == SYS_DVBC_ANNEX_A)
  {
		symbol_rate = (u32)(fe->dtv_property_cache.symbol_rate / 1000);
    DWORD_TO_BYTES(symbol_rate, cmd9, 20);
    
    switch ( fe->dtv_property_cache.modulation )
    {
      case QAM_16:
      	DWORD_TO_BYTES(16, cmd9, 24);
        break;
      case QAM_32:
      	DWORD_TO_BYTES(32, cmd9, 24);
        break;
      case QAM_64:
      	DWORD_TO_BYTES(64, cmd9, 24);
        break;
      case QAM_128:
      	DWORD_TO_BYTES(128, cmd9, 24);
        break;
      case QAM_256:
      	DWORD_TO_BYTES(256, cmd9, 24);
        break;
      default:
        break;
    }
  }
  else
  {
  	DWORD_TO_BYTES(0, cmd9, 20);
    DWORD_TO_BYTES(0, cmd9, 24);
    DWORD_TO_BYTES(8, cmd9, 28);
  }

  smit_set(priv->dev->udev, 0xA0u, cmd9, 32, 100);

  smit_tuner_stat(priv->dev->udev, buf, 2);

  priv->frequency = fe->dtv_property_cache.frequency;
  priv->symbol_rate = fe->dtv_property_cache.symbol_rate;
  priv->modulation = fe->dtv_property_cache.modulation;

  return 0;
}

static int icast_fe_get_tune_settings(struct dvb_frontend *fe,
			      struct dvb_frontend_tune_settings *fesettings)
{
	fesettings->min_delay_ms = 800;
	fesettings->step_size = 0;
	fesettings->max_drift = 0;
	return 0;
}

static int icast_fe_read_status(struct dvb_frontend *fe, enum fe_status *status)
{
  struct icast_state *priv = fe->demodulator_priv;
  u8 cmd12[20] = {
  	0x01, 0x90, 0x02, 0x00, 0x03, 0x9F, 0x9A, 0x07,
  	0x0B, 0x01, 0x00, 0x08, 0x00, 0x05, 0x00, 0x04,
  	0x47, 0x53, 0x54, 0x41
  };
  u8 buf[900];

  smit_set(priv->dev->udev, 0xA0u, cmd12, 20, 50);

  if ( smit_tuner_stat(priv->dev->udev, buf, 5) != 32 )
  {
    return 0;
  }
  priv->lock = (u8)buf[29];

  if ( buf[29] == 0 )
  {
    *status = 0;
  }
  else
  {
    *status = FE_HAS_SIGNAL | FE_HAS_LOCK;

		priv->frequency = (buf[19]<<24) + (buf[18]<<16) + (buf[17]<<8) + buf[16];
    priv->snr = (u8)buf[30];
    priv->strength = (u8)buf[31];
//    printk("buf[30]=%02X buf[31]=%02X", buf[30], buf[31]);
  }
  return 0;
}

static int icast_fe_read_signal_strength(struct dvb_frontend *fe, u16 *strength)
{
  struct icast_state *priv = fe->demodulator_priv;

  *strength = 0;
  if ( priv->lock )
    *strength = ((s64)((u64)(90070618176225 * (u8)priv->strength) >> 32) >> 5)
              - (0xFFFF * (u8)priv->strength >> 31);
  return 0;
}

static int icast_fe_read_snr(struct dvb_frontend *fe, u16 *snr)
{
  struct icast_state *priv = fe->demodulator_priv;

  *snr = 0;
  if ( priv->lock )
    *snr = ((s64)((u64)(90070618176225 * (u8)priv->snr) >> 32) >> 5)
         - (0xFFFF * (u8)priv->snr >> 31);
  return 0;
}

static const struct dvb_frontend_ops icast_fe_ops = {
	.delsys = { SYS_DVBT, SYS_DVBC_ANNEX_A },
	.info = {
		.name = "iCast DTMB/DVBC USB demodulator",
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
		.frequency_min = 10000000,
		.frequency_max = 862000000,
		.frequency_stepsize = 10000,
#else
		.frequency_min_hz	= 10 * MHz,
		.frequency_max_hz	= 862 * MHz,
		.frequency_stepsize_hz	= 10 * kHz,
#endif
		.caps =
			FE_CAN_FEC_AUTO |
			FE_CAN_QAM_AUTO |
			FE_CAN_TRANSMISSION_MODE_AUTO |
			FE_CAN_GUARD_INTERVAL_AUTO
	},

	.release = icast_fe_release,
	.init = icast_fe_init,
	.set_frontend = icast_fe_set_frontend,
	.get_frontend = icast_fe_get_frontend,
	.get_tune_settings = icast_fe_get_tune_settings,

	.read_status = icast_fe_read_status,
	.read_signal_strength = icast_fe_read_signal_strength,
	.read_snr = icast_fe_read_snr,
};

struct dvb_frontend *icast_fe_attach(struct dvb_usb_adapter *adap, const struct icast_fe_config *config)
{
	struct icast_state *priv = NULL;

	priv = kzalloc(sizeof(struct icast_state), GFP_KERNEL);
	if (priv == NULL)
		return NULL;
	
	memcpy(&priv->frontend.ops, &icast_fe_ops, sizeof(struct dvb_frontend_ops));
	
	priv->frontend.demodulator_priv = priv;
	priv->dev = adap->dev;
	
	return &priv->frontend;
}

EXPORT_SYMBOL_GPL(icast_fe_attach);

MODULE_DESCRIPTION("iCast frontend driver");
MODULE_AUTHOR("Xiaodong Ni <nxiaodong520@gmail.com>");
MODULE_LICENSE("GPL");

