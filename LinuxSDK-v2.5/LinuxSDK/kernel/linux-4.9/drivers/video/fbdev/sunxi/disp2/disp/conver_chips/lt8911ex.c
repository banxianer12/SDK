/* drivers/video/fbdev/sunxi/disp2/disp/conver_chips/lt8911ex.c
*
* Copyright (c) 2021 Allwinnertech Co., Ltd.
* Author: hongyaobin <hongyaobin@allwinnertech.com>
*
* LVDS to eDP (m133x56) driver
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*/

#include "lt8911ex.h"
#include <linux/i2c.h>

//#define CONVER_CHIPS_DEBUG
#ifdef CONVER_CHIPS_DEBUG
#define DEBUG(fmt, args...) printk("[DEBUG_MSG] " fmt, ## args)
#else
#define DEBUG(fmt, args...)
#endif

static struct i2c_client *this_client;

extern s32 bsp_disp_get_panel_info(u32 screen_id, struct disp_panel_para *info);
extern s32 bsp_disp_get_output_type(u32 disp);

static void lcd_power_on(u32 sel);
static void lcd_power_off(u32 sel);
static void lcd_bl_open(u32 sel);
static void lcd_bl_close(u32 sel);

static void lcd_panel_init(u32 sel);
static void lcd_panel_exit(u32 sel);

#define LT8911EX_RST	0

/***********************************************************************************

a)If the 13 pin (S_ADR) of LT8911EX is low, the IIC address of LT8911EX is 0x52;
bit0 is a read and write flag;if Linux system,  the bit 7 of the IIC address is
used as the read and write flag, then the IIC address should be 0x29.


b)If the 13 pin (S_ADR) of LT8911EX is high, the IIC address of LT8911EX is 0x5a;
bit0 is a read and write flag;if Linux system,  the bit 7 of the IIC address is
used as the read and write flag, then the IIC address should be 0x2d.

************************************************************************************/
#define LT8911EX_SLAVE_ADDR 0x29
#define LT8911EX_I2C_ID	0x05 //Twi5

#define panel_reset(sel, val) sunxi_lcd_gpio_set_value(sel, LT8911EX_RST, val)

static const struct of_device_id lt8911ex_match[] = {
	{.compatible = "allwinner,sunxi-disp",},
	{},
};

static  unsigned short normal_i2c[] = {LT8911EX_SLAVE_ADDR, I2C_CLIENT_END};

static const struct i2c_device_id lt8911ex_id[] = {
	{"LT8911EX", LT8911EX_I2C_ID}, /* name, private_data */
	{/* End of list */}
};
MODULE_DEVICE_TABLE(i2c, lt8911ex_id);

#define _1_Port_	0x80
#define _2_Port_	0x00
#define _JEIDA_ 	0x40
#define _VESA_		0x00
#define _Sync_Mode_ 0x00
#define _DE_Mode_	0x20

/*******************************************************************************************************/
#define _LVDS_Port_ _2_Port_
#define _LVDS_RX_ 	_VESA_
#define _LVDS_Mode_ _DE_Mode_
#define _eDP_2G7_

static int LVDS_Timing[] =
/* hfp, hs, hbp, hact, htotal, vfp, vs, vbp, vact, vtotal, pixel_CLK/10,000 */
{ 316, 32, 80, 1920, 2380, 11, 5, 24, 1080, 1125, 13900 };
/*
	the screen params of sunxi platform have some notices:
	lcd_hbp = hbp + hs
	lcd_vbp = vbp + vs
*/

//#define 	_8_to_6bit_Dither_En  	//if Input 8 bit LVDS , Output is 6 bit eDP
//#define	_6_bit_eDP_ 			//eDP panel Color Depth, 262K color
#define 	_8_bit_eDP_ 			//eDP panel Color Depth, 16.7M color
#define 	_eDP_Lane_ 	2

/* for test */
//#define _gpio_sync_output_

//#define _read_edid_
//#define _EDP_Pattern_ //LT8911EX test pattern Output
//#define sync_source 0x01    //gpio output lvds Rx sync
//#define sync_source 0x02 //gpio output lvds portA sync
//#define sync_source 0x03 //gpio output lvds portB sync

/*******************************************************************************************************/
u8 Swing_Setting1[] = {0x83, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x81, 0x80, 0x80, 0x80, 0x80};
u8 Swing_Setting2[] = {0x00, 0xe0, 0xc0, 0xa0, 0x80, 0x40, 0x20, 0x00, 0x00, 0xe0, 0xc0, 0xa0, 0x80};

enum {
	hfp = 0,
	hs,
	hbp,
	hact,
	htotal,
	vfp,
	vs,
	vbp,
	vact,
	vtotal,
	pclk_10khz
};

enum {
	_Level0_ = 0,   // 27.8 mA  0x83/0x00
	_Level1_,       // 26.2 mA  0x82/0xe0
	_Level2_,       // 24.6 mA  0x82/0xc0
	_Level3_,       // 23 mA    0x82/0xa0
	_Level4_,       // 21.4 mA  0x82/0x80
	_Level5_,       // 18.2 mA  0x82/0x40
	_Level6_,       // 16.6 mA  0x82/0x20
	_Level7_,       // 15mA     0x82/0x00
	_Level8_,       // 12.8mA   0x81/0x00
	_Level9_,       // 11.2mA   0x80/0xe0
	_Level10_,      // 9.6mA    0x80/0xc0
	_Level11_,      // 8mA      0x80/0xa0
	_Level12_,      // 6mA      0x80/0x80
};

#ifdef _read_edid_          //read eDP panel EDID

u8 EDID_DATA[128] = {0};
u16	EDID_Timing[11] = {0};

bool EDID_Reply;
#endif

u8 LVDS_Port;
u8 Level = _Level7_;
u8 Read_DPCD010A;
bool ScrambleMode;
u16 h_fp, h_sync, h_bp, h_active, h_total, v_fp, v_sync, v_bp, v_active, v_total, P_clk;

static u8 lt8911ex_Dpcd_read(u32 address)
{
	/***************************
	   注意大小端的问题!
	   这里默认是大端模式
	   Big-Endian
	 ****************************/
	u8 Dpcd_value = 0x00;
	u8 addressH = 0x0f & (address >> 16);
	u8 addressM = 0xff & (address >> 8);
	u8 addressL = 0xff & address;
	u8 reg;

	i2c_smbus_write_byte_data(this_client, 0xff, 0xac); //register bank
	i2c_smbus_write_byte_data(this_client, 0x00, 0x20); //Soft Link train
	i2c_smbus_write_byte_data(this_client, 0xff, 0xa6);
	i2c_smbus_write_byte_data(this_client, 0x2a, 0x01);

	i2c_smbus_write_byte_data(this_client, 0xff, 0xa6); //register bank
	i2c_smbus_write_byte_data(this_client, 0x2b, (0x90 | addressH)); //CMD
	i2c_smbus_write_byte_data(this_client, 0x2b, addressM); //addr[15:8]
	i2c_smbus_write_byte_data(this_client, 0x2b, addressL); //addr[7:0]
	i2c_smbus_write_byte_data(this_client, 0x2b, 0x00); //data lenth
	i2c_smbus_write_byte_data(this_client, 0x2c, 0x00); //start Aux read edid

	sunxi_lcd_delay_ms(50);
	reg = i2c_smbus_read_byte_data(this_client, 0x25);
	if ((reg & 0x0f) == 0x0c) {
		if (i2c_smbus_read_byte_data(this_client, 0x39) == 0x22) {
			i2c_smbus_read_byte_data(this_client, 0x2b);
			Dpcd_value = i2c_smbus_read_byte_data(this_client, 0x2b);
		}
	} else {
		//AUX reset
		i2c_smbus_write_byte_data(this_client, 0xff, 0x81); //change bank
		i2c_smbus_write_byte_data(this_client, 0x07, 0xfe);
		i2c_smbus_write_byte_data(this_client, 0x07, 0xff);
		i2c_smbus_write_byte_data(this_client, 0x0a, 0xfc);
		i2c_smbus_write_byte_data(this_client, 0x0a, 0xfe);
	}
	return Dpcd_value;
}

static u8 lt8911ex_Dpcd_write(u32 address, u8 data)
{
	/***************************
	   注意大小端的问题!
	   这里默认是大端模式
	   Big-Endian
	 ****************************/
	u8 addressH = 0x0f & (address >> 16);
	u8 addressM = 0xff & (address >> 8);
	u8 addressL = 0xff & address;
	u8 reg;

	i2c_smbus_write_byte_data(this_client, 0xff, 0xa6);
	i2c_smbus_write_byte_data(this_client, 0x2b, (0x80 | addressH)); //CMD
	i2c_smbus_write_byte_data(this_client, 0x2b, addressM); //addr[15:8]
	i2c_smbus_write_byte_data(this_client, 0x2b, addressL); //addr[7:0]
	i2c_smbus_write_byte_data(this_client, 0x2b, 0x00); //data lenth
	i2c_smbus_write_byte_data(this_client, 0x2b, data); //data
	i2c_smbus_write_byte_data(this_client, 0x2c, 0x00); //start Aux read edid
	sunxi_lcd_delay_ms(20); //more than 10ms
	reg = i2c_smbus_read_byte_data(this_client, 0x25);
	if ((reg & 0x0f) == 0x0c) {
		return 0;
	} else {
		DEBUG("%s (%u) lt8911ex_Dpcd_write error!\n", __FUNCTION__, __LINE__);
		return -1;
	}
}

void lt8911ex_read_edid(void)
{
#ifdef _read_edid_
	u8 reg, i, j;
	i2c_smbus_write_byte_data(this_client, 0xff, 0xac); //register bank
	i2c_smbus_write_byte_data(this_client, 0x00, 0x20); //Soft Link train

	i2c_smbus_write_byte_data(this_client, 0xff, 0xa6); //register bank
	i2c_smbus_write_byte_data(this_client, 0x2a, 0x01);

	/* set edid offset addr */
	i2c_smbus_write_byte_data(this_client, 0x2b, 0x40); //CMD
	i2c_smbus_write_byte_data(this_client, 0x2b, 0x00); //addr[15:8]
	i2c_smbus_write_byte_data(this_client, 0x2b, 0x50); //addr[7:0]
	i2c_smbus_write_byte_data(this_client, 0x2b, 0x00); //data lenth
	i2c_smbus_write_byte_data(this_client, 0x2b, 0x00); //data lenth
	i2c_smbus_write_byte_data(this_client, 0x2c, 0x00); //start Aux read edid

	DEBUG("%s (%u) Read eDP EDID......\n", __FUNCTION__, __LINE__);
	sunxi_lcd_delay_ms(20);
	reg = i2c_smbus_read_byte_data(this_client, 0x25);
	if ((reg & 0x0f) == 0x0c) {
		for (j = 0; j < 8; j++) {
			if (j == 7)
				i2c_smbus_write_byte_data(this_client, 0x2b, 0x10);
			else
				i2c_smbus_write_byte_data(this_client, 0x2b, 0x50);

			i2c_smbus_write_byte_data(this_client, 0x2b, 0x00);
			i2c_smbus_write_byte_data(this_client, 0x2b, 0x50);
			i2c_smbus_write_byte_data(this_client, 0x2b, 0x0f);
			i2c_smbus_write_byte_data(this_client, 0x2c, 0x00); //start Aux read edid
			sunxi_lcd_delay_ms(50);

			if (i2c_smbus_read_byte_data(this_client, 0x39) == 0x31) {
				i2c_smbus_read_byte_data(this_client, 0x2b);
				for (i = 0; i < 16; i++)
					EDID_DATA[j * 16 + i] = i2c_smbus_read_byte_data(this_client, 0x2b);

				EDID_Reply = 1;
			} else {
				EDID_Reply = 0;
				DEBUG("[DEBUG_MSG] %s (%u) no_reply\n", __FUNCTION__, __LINE__);
				return;
			}
		}

		EDID_Timing[hfp] = ((EDID_DATA[0x41] & 0xC0) * 4 + EDID_DATA[0x3e]);
		EDID_Timing[hs] = ((EDID_DATA[0x41] & 0x30) * 16 + EDID_DATA[0x3f]);
		EDID_Timing[hbp] = (((EDID_DATA[0x3a] & 0x0f) * 0x100 + EDID_DATA[0x39]) - ((EDID_DATA[0x41] & 0x30) \
							* 16 + EDID_DATA[0x3f]) - ((EDID_DATA[0x41] & 0xC0) * 4 + EDID_DATA[0x3e]));
		EDID_Timing[hact] = ((EDID_DATA[0x3a] & 0xf0) * 16 + EDID_DATA[0x38]);
		EDID_Timing[htotal] = ((EDID_DATA[0x3a] & 0xf0) * 16 + EDID_DATA[0x38] + ((EDID_DATA[0x3a] & 0x0f) \
							* 0x100 + EDID_DATA[0x39]));
		EDID_Timing[vfp] = ((EDID_DATA[0x41] & 0x0c) * 4 + (EDID_DATA[0x40] & 0xf0) / 16);
		EDID_Timing[vs] = ((EDID_DATA[0x41] & 0x03) * 16 + EDID_DATA[0x40] & 0x0f);
		EDID_Timing[vbp] = (((EDID_DATA[0x3d] & 0x03) * 0x100 + EDID_DATA[0x3c]) - ((EDID_DATA[0x41] & 0x03)
							* 16 + EDID_DATA[0x40] & 0x0f) - ((EDID_DATA[0x41] & 0x0c) * 4 + (EDID_DATA[0x40] \
							& 0xf0) / 16));
		EDID_Timing[vact] = ((EDID_DATA[0x3d] & 0xf0) * 16 + EDID_DATA[0x3b]);
		EDID_Timing[vtotal] = ((EDID_DATA[0x3d] & 0xf0) * 16 + EDID_DATA[0x3b] + ((EDID_DATA[0x3d] & 0x0) \
							* 0x100 + EDID_DATA[0x3c]));
		EDID_Timing[pclk_10khz] = (EDID_DATA[0x37] * 0x100 + EDID_DATA[0x36]);

		DEBUG("%s (%u) H_FP / H_pluse / H_BP / H_act / H_tol / V_FP / V_pluse / V_BP / V_act / V_tol / D_CLK\n", __FUNCTION__, __LINE__);
		DEBUG(" %d / %d / %d / %d / %d / %d / %d / %d / %d / %d \n", EDID_Timing[hfp], EDID_Timing[hs], EDID_Timing[hbp], \
				EDID_Timing[hact], EDID_Timing[htotal], EDID_Timing[vfp], EDID_Timing[vs], EDID_Timing[vbp], EDID_Timing[vact], \
				EDID_Timing[vtotal], EDID_Timing[pclk_10khz]);
		} else
			DEBUG("%s (%u) read eDP EDID failed!\n", __FUNCTION__, __LINE__);
#endif
}

void lt8911ex_link_train(void)
{
	i2c_smbus_write_byte_data(this_client, 0xff, 0x85);
	if (ScrambleMode) {
		i2c_smbus_write_byte_data(this_client, 0xa1, 0x81); //eDP scramble mode

		/* Aux operater init */
		i2c_smbus_write_byte_data(this_client, 0xff, 0xac);
		i2c_smbus_write_byte_data(this_client, 0x00, 0x20); //Soft Link train
		i2c_smbus_write_byte_data(this_client, 0xff, 0xa6);
		i2c_smbus_write_byte_data(this_client, 0x2a, 0x01);

		lt8911ex_Dpcd_write(0x010a, 0x01);
		sunxi_lcd_delay_ms(10);
		lt8911ex_Dpcd_write(0x0102, 0x00);
		sunxi_lcd_delay_ms(10);
		lt8911ex_Dpcd_write(0x010a, 0x01);
		sunxi_lcd_delay_ms(200);
	} else
		i2c_smbus_write_byte_data(this_client, 0xa1, 0x01); //DP scramble mode

	/* Aux setup */
	i2c_smbus_write_byte_data(this_client, 0xff, 0xac); //register bank
	i2c_smbus_write_byte_data(this_client, 0x00, 0x60);

	i2c_smbus_write_byte_data(this_client, 0xff, 0xa6); //register bank
	i2c_smbus_write_byte_data(this_client, 0x2a, 0x00);

	i2c_smbus_write_byte_data(this_client, 0xff, 0x81); //register bank
	i2c_smbus_write_byte_data(this_client, 0x07, 0xfe);
	i2c_smbus_write_byte_data(this_client, 0x07, 0xff);
	i2c_smbus_write_byte_data(this_client, 0x0a, 0xfc);
	i2c_smbus_write_byte_data(this_client, 0x0a, 0xfe);

#ifdef _EDP_Pattern_ //test pattern Display
	i2c_smbus_write_byte_data(this_client, 0xff, 0xa8);
	i2c_smbus_write_byte_data(this_client, 0x2d, 0x88);
#else
	i2c_smbus_write_byte_data(this_client, 0xff, 0xa8);
#ifdef _Msa_Active_Only_
	i2c_smbus_write_byte_data(this_client, 0x2d, 0x80);
#else
	i2c_smbus_write_byte_data(this_client, 0x2d, 0x00);
#endif

#endif

	/* link train */
	i2c_smbus_write_byte_data(this_client, 0xff, 0x85);
	i2c_smbus_write_byte_data(this_client, 0x1a, _eDP_Lane_);

	i2c_smbus_write_byte_data(this_client, 0xff, 0xac);
	i2c_smbus_write_byte_data(this_client, 0x00, 0x64);
	i2c_smbus_write_byte_data(this_client, 0x01, 0x0a);
	i2c_smbus_write_byte_data(this_client, 0x0c, 0x85);
	i2c_smbus_write_byte_data(this_client, 0x0c, 0xc5);
}

void lt8911ex_link_train_result(void)
{
#ifdef CONVER_CHIPS_DEBUG
	u8 i;
	u8 val;

	i2c_smbus_write_byte_data(this_client, 0xff, 0xac);
	for (i = 0; i < 10; i++) {
		val = i2c_smbus_read_byte_data(this_client, 0x82);
		if (val & 0x20) {
			if ((val & 0x1f) == 0x1e)
				DEBUG("%s (%u) edp link train successed: 0x%x!\n", __FUNCTION__, __LINE__, val);
			else
				DEBUG("%s (%u) edp link train failed: 0x%x!\n", __FUNCTION__, __LINE__, val);

			val = i2c_smbus_read_byte_data(this_client, 0x83);
			DEBUG("%s (%u) panel link rate: 0x%x\n", __FUNCTION__, __LINE__, val);
			val = i2c_smbus_read_byte_data(this_client, 0x84);
			DEBUG("%s (%u) panel link count: 0x%x\n", __FUNCTION__, __LINE__, val);

			return;
		} else
			DEBUG("%s (%u) link trian on going...\n", __FUNCTION__, __LINE__);
		sunxi_lcd_delay_ms(100);
	}
	DEBUG("%s (%u) finally, link trian on failed!\n", __FUNCTION__, __LINE__);
#endif
}

static s32 lt8911ex_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	this_client = client;
	return 0;
}

static s32 lt8911ex_i2c_remove(struct i2c_client *client)
{
	return 0;
}

static s32 lt8911ex_i2c_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	const char *type_name = "LT8911EX";
	DEBUG("%s (%u) lt8911ex_i2c_detect\n", __FUNCTION__, __LINE__);
	if (LT8911EX_I2C_ID == client->adapter->nr) {
		strlcpy(info->type, type_name, 20);
	} else
		pr_warn("%s:%d wrong i2c id:%d, expect id is :%d\n", __func__, __LINE__,
			client->adapter->nr, LT8911EX_I2C_ID);
	return 0;
}

static struct i2c_driver lt8911ex_i2c_driver = {
	.class = I2C_CLASS_HWMON,
	.id_table = lt8911ex_id,
	.probe = lt8911ex_i2c_probe,
	.remove = lt8911ex_i2c_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "LT8911EX",
		.of_match_table = lt8911ex_match,
	},
	.detect = lt8911ex_i2c_detect,
	.address_list = normal_i2c,
};

static void lt8911ex_chipID_read(void)
{
#ifdef CONVER_CHIPS_DEBUG
	//Read LT8911EX_ChipID，just for debug
	u8 chip_ID[3] = {0};
	i2c_smbus_write_byte_data(this_client, 0xff, 0x81); //register bank
	i2c_smbus_write_byte_data(this_client, 0x08, 0x7f);
	chip_ID[2] = i2c_smbus_read_byte_data(this_client, 0x00);
	chip_ID[1] = i2c_smbus_read_byte_data(this_client, 0x01);
	chip_ID[0] = i2c_smbus_read_byte_data(this_client, 0x02);
	DEBUG("*********************************************\n");
	DEBUG("Chip ID high byte = 0x%x\n", chip_ID[2]);
	DEBUG("Chip ID middle byte = 0x%x\n", chip_ID[1]);
	DEBUG("Chip ID low byte = 0x%x\n", chip_ID[0]);
	DEBUG("*********************************************\n");
#endif
}

void lt8911ex_eDP_video_cfg(void)
{
	u8 dessc_m;
	i2c_smbus_write_byte_data(this_client, 0xff, 0xa8); //register bank

#ifdef _Msa_Active_Only_
	i2c_smbus_write_byte_data(this_client, 0x2d, 0x84);	//bit[7]：1 = register msa, 0 = hardware msa
	i2c_smbus_write_byte_data(this_client, 0x05, 0x00);
	i2c_smbus_write_byte_data(this_client, 0x06, 0x00);	//htotal
	i2c_smbus_write_byte_data(this_client, 0x07, 0x00);
	i2c_smbus_write_byte_data(this_client, 0x08, 0x00);	//h_start
	i2c_smbus_write_byte_data(this_client, 0x09, 0x00);
	i2c_smbus_write_byte_data(this_client, 0x0a, 0x00);	//hsa
	i2c_smbus_write_byte_data(this_client, 0x0b, (u8)(LVDS_Timing[hact] / 256));
	i2c_smbus_write_byte_data(this_client, 0x0c, (u8)(LVDS_Timing[hact] % 256)); //hactive
	i2c_smbus_write_byte_data(this_client, 0x0d, 0x00);
	i2c_smbus_write_byte_data(this_client, 0x0e, 0x00);	//vtotal
	i2c_smbus_write_byte_data(this_client, 0x11, 0x00);
	i2c_smbus_write_byte_data(this_client, 0x12, 0x00);
	i2c_smbus_write_byte_data(this_client, 0x14, 0x00);
	i2c_smbus_write_byte_data(this_client, 0x15, (u8)(LVDS_Timing[vact] / 256));
	i2c_smbus_write_byte_data(this_client, 0x16, (u8)(LVDS_Timing[vact] % 256));   //vactive
#else
	i2c_smbus_write_byte_data(this_client, 0x2d, 0x04); //0xa82d, bit[7]：1 = register msa, 0 = hardware msa
	i2c_smbus_write_byte_data(this_client, 0x05, (u8)(LVDS_Timing[htotal] / 256));
	i2c_smbus_write_byte_data(this_client, 0x06, (u8)(LVDS_Timing[htotal] % 256));
	i2c_smbus_write_byte_data(this_client, 0x07, (u8)((LVDS_Timing[hs] + LVDS_Timing[hbp]) / 256));
	i2c_smbus_write_byte_data(this_client, 0x08, (u8)((LVDS_Timing[hs] + LVDS_Timing[hbp]) % 256));
	i2c_smbus_write_byte_data(this_client, 0x09, (u8)(LVDS_Timing[hs] / 256));
	i2c_smbus_write_byte_data(this_client, 0x0a, (u8)(LVDS_Timing[hs] % 256));
	i2c_smbus_write_byte_data(this_client, 0x0b, (u8)(LVDS_Timing[hact] / 256));
	i2c_smbus_write_byte_data(this_client, 0x0c, (u8)(LVDS_Timing[hact] % 256));
	i2c_smbus_write_byte_data(this_client, 0x0d, (u8)(LVDS_Timing[vtotal] / 256));
	i2c_smbus_write_byte_data(this_client, 0x0e, (u8)(LVDS_Timing[vtotal] % 256));
	i2c_smbus_write_byte_data(this_client, 0x11, (u8)((LVDS_Timing[vs] + LVDS_Timing[vbp]) / 256));
	i2c_smbus_write_byte_data(this_client, 0x12, (u8)((LVDS_Timing[vs] + LVDS_Timing[vbp]) % 256));
	i2c_smbus_write_byte_data(this_client, 0x14, (u8)(LVDS_Timing[vs] % 256));
	i2c_smbus_write_byte_data(this_client, 0x15, (u8)(LVDS_Timing[vact] / 256));
	i2c_smbus_write_byte_data(this_client, 0x16, (u8)(LVDS_Timing[vact] % 256));
#endif
	//LVDS de only mode to regenerate h/v sync.
	i2c_smbus_write_byte_data(this_client, 0xff, 0xd8); //register bank
	i2c_smbus_write_byte_data(this_client, 0x20, (u8)(LVDS_Timing[hfp] / 256));
	i2c_smbus_write_byte_data(this_client, 0x21, (u8)(LVDS_Timing[hfp] % 256));

	i2c_smbus_write_byte_data(this_client, 0x22, (u8)(LVDS_Timing[hs] / 256));
	i2c_smbus_write_byte_data(this_client, 0x23, (u8)(LVDS_Timing[hs] % 256));

	i2c_smbus_write_byte_data(this_client, 0x24, (u8)(LVDS_Timing[htotal] / 256));
	i2c_smbus_write_byte_data(this_client, 0x25, (u8)(LVDS_Timing[htotal] % 256));

	i2c_smbus_write_byte_data(this_client, 0x26, (u8)(LVDS_Timing[vfp] % 256));
	i2c_smbus_write_byte_data(this_client, 0x27, (u8)(LVDS_Timing[vs] % 256));

	dessc_m = (LVDS_Timing[pclk_10khz] * 4) / (25 * 100);

	i2c_smbus_write_byte_data(this_client, 0xff, 0x85); // register bank
	i2c_smbus_write_byte_data(this_client, 0xaa, dessc_m);
}

static void lt8911ex_reset(u32 sel)
{
	panel_reset(sel, 0); //GPIO LOW
	sunxi_lcd_delay_ms(100);
	panel_reset(sel, 1); //GPIO high
	sunxi_lcd_delay_ms(100);
}

static void lt8911ex_lvds_clock_check(void)
{
	u32 fm_value = 0;

	i2c_smbus_write_byte_data(this_client, 0xff, 0x85);
	i2c_smbus_write_byte_data(this_client, 0x1d, 0x03);
	sunxi_lcd_delay_ms(200);
	fm_value = i2c_smbus_read_byte_data(this_client, 0x4d);
	fm_value = fm_value * 0x100 + i2c_smbus_read_byte_data(this_client, 0x4e);
	fm_value = fm_value * 0x100 + i2c_smbus_read_byte_data(this_client, 0x4f);
	DEBUG("%s (%u) lvds portA clock: %d \n", __FUNCTION__, __LINE__, fm_value);

	i2c_smbus_write_byte_data(this_client, 0x1d, 0x04);
	sunxi_lcd_delay_ms(200);
	fm_value = 0;
	fm_value = i2c_smbus_read_byte_data(this_client, 0x4d);
	fm_value = fm_value * 0x100 + i2c_smbus_read_byte_data(this_client, 0x4e);
	fm_value = fm_value * 0x100 + i2c_smbus_read_byte_data(this_client, 0x4f);
	DEBUG("%s (%u) lvds portB clock: %d \n", __FUNCTION__, __LINE__, fm_value);

	i2c_smbus_write_byte_data(this_client, 0x1d, 0x08);
	sunxi_lcd_delay_ms(200);
	fm_value = 0;
	fm_value = i2c_smbus_read_byte_data(this_client, 0x4d);
	fm_value = fm_value * 0x100 + i2c_smbus_read_byte_data(this_client, 0x4e);
	fm_value = fm_value * 0x100 + i2c_smbus_read_byte_data(this_client, 0x4f);
	DEBUG("%s (%u) lvds pixel clock: %d \n", __FUNCTION__, __LINE__, fm_value);
}

static void lt8911ex_lvds_video_check(void)
{
	u8 sync_polarity = 0;
	i2c_smbus_write_byte_data(this_client, 0xff, 0x85);
	if (ScrambleMode)
		i2c_smbus_write_byte_data(this_client, 0xa1, 0x81); //eDP scramble mode;
	else
		i2c_smbus_write_byte_data(this_client, 0xa1, 0x01); //DP scramble mode;

	sync_polarity = i2c_smbus_read_byte_data(this_client, 0x7a);

	v_sync = i2c_smbus_read_byte_data(this_client, 0x7b);

	h_sync = i2c_smbus_read_byte_data(this_client, 0x7c);
	h_sync = h_sync * 0x100 + i2c_smbus_read_byte_data(this_client, 0x7d);

	v_bp   = i2c_smbus_read_byte_data(this_client, 0x7e);
	v_fp   = i2c_smbus_read_byte_data(this_client, 0x7f);

	h_bp   = i2c_smbus_read_byte_data(this_client, 0x80);
	h_bp   = h_bp * 0x100 + i2c_smbus_read_byte_data(this_client, 0x81);

	h_fp   = i2c_smbus_read_byte_data(this_client, 0x82);
	h_fp   = h_fp * 0x100 + i2c_smbus_read_byte_data(this_client, 0x83);

	v_total	   = i2c_smbus_read_byte_data(this_client, 0x84);
	v_total	   = v_total * 0x100 + i2c_smbus_read_byte_data(this_client, 0x85);

	h_total	   = i2c_smbus_read_byte_data(this_client, 0x86);
	h_total	   = h_total * 0x100 + i2c_smbus_read_byte_data(this_client, 0x87);

	v_active   = i2c_smbus_read_byte_data(this_client, 0x88);
	v_active   = v_active * 0x100 + i2c_smbus_read_byte_data(this_client, 0x89);

	h_active   = i2c_smbus_read_byte_data(this_client, 0x8a);
	h_active   = h_active * 0x100 + i2c_smbus_read_byte_data(this_client, 0x8b);

	DEBUG("%s (%u) hfp, hs, hbp, hact, htotal = %d, %d, %d, %d, %d \n", \
			__FUNCTION__, __LINE__, h_fp, h_sync, h_bp, h_active, h_total);

	DEBUG("%s (%u) vfp, vs, vbp, vact, vtotal = %d, %d, %d, %d, %d \n", \
			__FUNCTION__, __LINE__, v_fp, v_sync, v_bp, v_active, v_total);
}

void lt8911ex_txswing_preset(void)
{
	i2c_smbus_write_byte_data(this_client, 0xff, 0x82);
	i2c_smbus_write_byte_data(this_client, 0x22, Swing_Setting1[Level]); //lane 0 tap0
	i2c_smbus_write_byte_data(this_client, 0x23, Swing_Setting2[Level]);
	i2c_smbus_write_byte_data(this_client, 0x24, 0x80);	//lane 0 tap1
	i2c_smbus_write_byte_data(this_client, 0x25, 0x00);
	i2c_smbus_write_byte_data(this_client, 0x26, Swing_Setting1[Level]); //lane 1 tap0
	i2c_smbus_write_byte_data(this_client, 0x27, Swing_Setting2[Level]);
	i2c_smbus_write_byte_data(this_client, 0x28, 0x80); //lane 1 tap1
	i2c_smbus_write_byte_data(this_client, 0x29, 0x00);
}

static void lt8911ex_init(void)
{
	LVDS_Port = _LVDS_Port_;

	/* init */
	i2c_smbus_write_byte_data(this_client, 0xff, 0x81); //register bank
	i2c_smbus_write_byte_data(this_client, 0x49, 0xff);
#ifdef _eDP_2G7_
	/* Txpll 2.7G*/
	i2c_smbus_write_byte_data(this_client, 0xff, 0x87); //register bank
	i2c_smbus_write_byte_data(this_client, 0x19, 0x31);
	i2c_smbus_write_byte_data(this_client, 0x1a, 0x36); //sync m
	i2c_smbus_write_byte_data(this_client, 0x1b, 0x00); //sync_k [7:0]
	i2c_smbus_write_byte_data(this_client, 0x1c, 0x00); //sync_k [13:8]

	//txpll Analog
	i2c_smbus_write_byte_data(this_client, 0xff, 0x82); //register bank
	i2c_smbus_write_byte_data(this_client, 0x02, 0x42);
	i2c_smbus_write_byte_data(this_client, 0x03, 0x00); //txpll en = 0
	i2c_smbus_write_byte_data(this_client, 0x03, 0x01); //txpll en = 1

	i2c_smbus_write_byte_data(this_client, 0xff, 0x87); //register bank
	i2c_smbus_write_byte_data(this_client, 0x0c, 0x10); //cal en = 0

	i2c_smbus_write_byte_data(this_client, 0xff, 0x81); //register bank
	i2c_smbus_write_byte_data(this_client, 0x09, 0xfc);
	i2c_smbus_write_byte_data(this_client, 0x09, 0xfd);

	i2c_smbus_write_byte_data(this_client, 0xff, 0x87); //register bank
	i2c_smbus_write_byte_data(this_client, 0x0c, 0x11); //cal en = 1

	//ssc
	i2c_smbus_write_byte_data(this_client, 0xff, 0x87); //register bank
	i2c_smbus_write_byte_data(this_client, 0x13, 0x83);
	i2c_smbus_write_byte_data(this_client, 0x14, 0x41);
	i2c_smbus_write_byte_data(this_client, 0x16, 0x0a);
	i2c_smbus_write_byte_data(this_client, 0x18, 0x0a);
	i2c_smbus_write_byte_data(this_client, 0x19, 0x33);
#endif

#ifdef _eDP_1G62_
	i2c_smbus_write_byte_data(this_client, 0xff, 0x87); //register bank
	i2c_smbus_write_byte_data(this_client, 0x19, 0x31);
	i2c_smbus_write_byte_data(this_client, 0x1a, 0x20); //sync m
	i2c_smbus_write_byte_data(this_client, 0x1b, 0x19); //sync_k [7:0]
	i2c_smbus_write_byte_data(this_client, 0x1c, 0x99); //sync_k [13:8]

	//txpll Analog
	i2c_smbus_write_byte_data(this_client, 0xff, 0x82); //register bank
	i2c_smbus_write_byte_data(this_client, 0x02, 0x42);
	i2c_smbus_write_byte_data(this_client, 0x03, 0x00); //txpll en = 0
	i2c_smbus_write_byte_data(this_client, 0x03, 0x01); //txpll en = 1

	i2c_smbus_write_byte_data(this_client, 0xff, 0x87); //register bank
	i2c_smbus_write_byte_data(this_client, 0x0c, 0x10); //cal en = 0

	i2c_smbus_write_byte_data(this_client, 0xff, 0x81); //register bank
	i2c_smbus_write_byte_data(this_client, 0x09, 0xfc);
	i2c_smbus_write_byte_data(this_client, 0x09, 0xfd);
							  this_client,
	i2c_smbus_write_byte_data(this_client, 0xff, 0x87); //register bank
	i2c_smbus_write_byte_data(this_client, 0x0c, 0x11); //cal en = 1

	//ssc
	i2c_smbus_write_byte_data(this_client, 0xff, 0x87); //register bank
	i2c_smbus_write_byte_data(this_client, 0x13, 0x83);
	i2c_smbus_write_byte_data(this_client, 0x14, 0x41);
	i2c_smbus_write_byte_data(this_client, 0x16, 0x0a);
	i2c_smbus_write_byte_data(this_client, 0x18, 0x0a);
	i2c_smbus_write_byte_data(this_client, 0x19, 0x33);
#endif

	/* lvds Rx analog */
	i2c_smbus_write_byte_data(this_client, 0xff, 0x82); //register bank
	i2c_smbus_write_byte_data(this_client, 0x3e, 0xa1);

	if (LVDS_Port == _1_Port_) {
		i2c_smbus_write_byte_data(this_client, 0xff, 0x82); //register bank
		i2c_smbus_write_byte_data(this_client, 0x63, 0x84);
		i2c_smbus_write_byte_data(this_client, 0x67, 0x40);
	}

	/* dessc/pcr  pll- analog */
#ifdef _EDP_Pattern_
	i2c_smbus_write_byte_data(this_client, 0xff, 0xd0);  //register bank
	i2c_smbus_write_byte_data(this_client, 0x08, 0x08);

	i2c_smbus_write_byte_data(this_client, 0xff, 0x82);  //register bank
	i2c_smbus_write_byte_data(this_client, 0x6e, 0x81);
	i2c_smbus_write_byte_data(this_client, 0x6a, 0x40);  //0x50:Pattern; 0x10:mipi video

	/* dessc pll digital */
	i2c_smbus_write_byte_data(this_client, 0xff, 0x85);  //register bank
	i2c_smbus_write_byte_data(this_client, 0xa9, 0x31);  //bit[0] = 1 : select software MK.
	i2c_smbus_write_byte_data(this_client, 0xae, 0x01);  //load MK value
	i2c_smbus_write_byte_data(this_client, 0xae, 0x11);
#endif

	/* digital top */
	i2c_smbus_write_byte_data(this_client, 0xff, 0x85);  //register bank

#ifdef _8_to_6bit_Dither_En
	i2c_smbus_write_byte_data(this_client, 0xb0, 0xd0);
#else
	i2c_smbus_write_byte_data(this_client, 0xb0, 0x00);
#endif

	/* lvds digital */
	i2c_smbus_write_byte_data(this_client, 0xff, 0x85); //register bank
	i2c_smbus_write_byte_data(this_client, 0xc3, 0x20); //portB LVDS clk from portA

	i2c_smbus_write_byte_data(this_client, 0xff, 0x81);
	i2c_smbus_write_byte_data(this_client, 0x05, 0xae);
	i2c_smbus_write_byte_data(this_client, 0x05, 0xfe);

	/* AUX reset */
	i2c_smbus_write_byte_data(this_client, 0xff, 0x81); //register bank
	i2c_smbus_write_byte_data(this_client, 0x07, 0xfe);
	i2c_smbus_write_byte_data(this_client, 0x07, 0xff);
	i2c_smbus_write_byte_data(this_client, 0x0a, 0xfc);
	i2c_smbus_write_byte_data(this_client, 0x0a, 0xfe);

	i2c_smbus_write_byte_data(this_client, 0xff, 0xd8);
	i2c_smbus_write_byte_data(this_client, 0x10, _LVDS_Port_);
	i2c_smbus_write_byte_data(this_client, 0x11, _LVDS_RX_);
	i2c_smbus_write_byte_data(this_client, 0x1f, _LVDS_Mode_);

	/* tx phy */
	i2c_smbus_write_byte_data(this_client, 0xff, 0x82); //register bank
	i2c_smbus_write_byte_data(this_client, 0x11, 0x00);
	i2c_smbus_write_byte_data(this_client, 0x13, 0x10);
	i2c_smbus_write_byte_data(this_client, 0x14, 0x0c);
	i2c_smbus_write_byte_data(this_client, 0x14, 0x08);
	i2c_smbus_write_byte_data(this_client, 0x13, 0x20);

	i2c_smbus_write_byte_data(this_client, 0xff, 0x82); //register bank
	i2c_smbus_write_byte_data(this_client, 0x0e, 0x35);
	i2c_smbus_write_byte_data(this_client, 0x12, 0x33);

	/*eDP Tx Digital */
	i2c_smbus_write_byte_data(this_client, 0xff, 0xa8); //register bank

#ifdef _EDP_Pattern_
	i2c_smbus_write_byte_data(this_client, 0x24, 0x50); //bit2 ~ bit 0 : test panttern image mode
	i2c_smbus_write_byte_data(this_client, 0x25, 0x70); //bit6 ~ bit 4 : test Pattern color
	i2c_smbus_write_byte_data(this_client, 0x27, 0x50); //0x50:Pattern; 0x10:mipi video
#else
	i2c_smbus_write_byte_data(this_client, 0x27, 0x10);
#endif

#ifdef _6_bit_eDP
	i2c_smbus_write_byte_data(this_client, 0x17, 0x00);
	i2c_smbus_write_byte_data(this_client, 0x18, 0x00);
#else //8 bit
	i2c_smbus_write_byte_data(this_client, 0x17, 0x10);
	i2c_smbus_write_byte_data(this_client, 0x18, 0x20);
#endif

	/* nvid */
	i2c_smbus_write_byte_data(this_client, 0xff, 0xa0);
	i2c_smbus_write_byte_data(this_client, 0x00, 0x08);
	i2c_smbus_write_byte_data(this_client, 0x01, 0x00);

#ifdef _INT_En_
	/* irq: lvds_clk_chg */
	i2c_smbus_write_byte_data(this_client, 0xff, 0x85); //register bank
	i2c_smbus_write_byte_data(this_client, 0x41, 0x3c);

	i2c_smbus_write_byte_data(this_client, 0x02, 0x7f); //mask: bit[7] vid_chk
	i2c_smbus_write_byte_data(this_client, 0x03, 0x3f); //mask: bit[7:6]

	i2c_smbus_write_byte_data(this_client, 0x07, 0xff);
	i2c_smbus_write_byte_data(this_client, 0x08, 0xff);

	/* interrupt */
	i2c_smbus_write_byte_data(this_client, 0xff, 0x82); //register bank
	i2c_smbus_write_byte_data(this_client, 0x5f, 0x00); //enable IRQ
#endif

#ifdef sync_source
	/* gpio init */
	i2c_smbus_write_byte_data(this_client, 0xff, 0xd8); //register bank
	i2c_smbus_write_byte_data(this_client, 0x16, sync_source); //lvds gpio test select: 0x02 port A, 0x03 portB, 0x01 lvds Rx
#endif

	i2c_smbus_write_byte_data(this_client, 0xff, 0x85); //register bank
	i2c_smbus_write_byte_data(this_client, 0x1b, 0x05); //gpio0: irq; gpio1: hpd; gpio2: test2; gpio3: test3; gpio4:test4
	i2c_smbus_write_byte_data(this_client, 0xc0, 0x00); //test2: de
	i2c_smbus_write_byte_data(this_client, 0xc1, 0x24); //test3: vs, test4：hs

	i2c_smbus_write_byte_data(this_client, 0xff, 0x82); //register bank
	i2c_smbus_write_byte_data(this_client, 0x5a, 0x01); //disable atest
	i2c_smbus_write_byte_data(this_client, 0x5b, 0xc0); //gpio0/1: oppen drain; gpio2/3 pp;

#ifdef _gpio_sync_output_
	i2c_smbus_write_byte_data(this_client, 0x60, 0x00); //gpio2/3: test mode
	i2c_smbus_write_byte_data(this_client, 0x61, 0x00); //gpio4: test mode
#else
	i2c_smbus_write_byte_data(this_client, 0x60, 0x14); //gpio2/3: not test mode
	i2c_smbus_write_byte_data(this_client, 0x61, 0x80); //gpio4: not test mode
#endif
}

static void lcd_cfg_panel_info(struct panel_extend_para *info)
{
	u32 i = 0, j = 0;
	u32 items;
	u8 lcd_gamma_tbl[][2] = {
		{0, 0},
		{15, 15},
		{30, 30},
		{45, 45},
		{60, 60},
		{75, 75},
		{90, 90},
		{105, 105},
		{120, 120},
		{135, 135},
		{150, 150},
		{165, 165},
		{180, 180},
		{195, 195},
		{210, 210},
		{225, 225},
		{240, 240},
		{255, 255},
	};

	u32 lcd_cmap_tbl[2][3][4] = {
		{
			{LCD_CMAP_G0, LCD_CMAP_B1, LCD_CMAP_G2, LCD_CMAP_B3},
			{LCD_CMAP_B0, LCD_CMAP_R1, LCD_CMAP_B2, LCD_CMAP_R3},
			{LCD_CMAP_R0, LCD_CMAP_G1, LCD_CMAP_R2, LCD_CMAP_G3},
		},
		{
			{LCD_CMAP_B3, LCD_CMAP_G2, LCD_CMAP_B1, LCD_CMAP_G0},
			{LCD_CMAP_R3, LCD_CMAP_B2, LCD_CMAP_R1, LCD_CMAP_B0},
			{LCD_CMAP_G3, LCD_CMAP_R2, LCD_CMAP_G1, LCD_CMAP_R0},
		},
	};

	items = sizeof(lcd_gamma_tbl) / 2;
	for (i = 0; i < items - 1; i++) {
		u32 num = lcd_gamma_tbl[i + 1][0] - lcd_gamma_tbl[i][0];

		for (j = 0; j < num; j++) {
			u32 value = 0;
			value = lcd_gamma_tbl[i][1] + ((lcd_gamma_tbl[i+1][1] - lcd_gamma_tbl[i][1]) * j) / num;
			info->lcd_gamma_tbl[lcd_gamma_tbl[i][0] + j] = (value<<16) + (value<<8) + value;
		}
	}
	info->lcd_gamma_tbl[255] = (lcd_gamma_tbl[items - 1][1] << 16) +
					(lcd_gamma_tbl[items - 1][1] << 8)
					+ lcd_gamma_tbl[items - 1][1];

	memcpy(info->lcd_cmap_tbl, lcd_cmap_tbl, sizeof(lcd_cmap_tbl));

}


static s32 lcd_open_flow(u32 sel)
{
	LCD_OPEN_FUNC(sel, lcd_power_on, 10);
	LCD_OPEN_FUNC(sel, sunxi_lcd_tcon_enable, 220);
	LCD_OPEN_FUNC(sel, lcd_panel_init, 50);
	LCD_OPEN_FUNC(sel, lcd_bl_open, 0);

	return 0;
}

static s32 lcd_close_flow(u32 sel)
{
	LCD_CLOSE_FUNC(sel, lcd_bl_close, 220);
	LCD_CLOSE_FUNC(sel, sunxi_lcd_tcon_disable, 10);
	LCD_CLOSE_FUNC(sel, lcd_panel_exit,	100);
	LCD_CLOSE_FUNC(sel, lcd_power_off, 0);

	return 0;
}

static void lcd_power_on(u32 sel)
{
	/* 打开/关闭 lcd 电源，操作的是board.dts中的lcd_power/lcd_power1/lcd_power2 */
	sunxi_lcd_power_enable(sel, 0);

	lt8911ex_reset(sel); //复位LT8911EX

	sunxi_lcd_pin_cfg(sel, 1); //配置lcd相关的gpio
}

static void lcd_power_off(u32 sel)
{
	sunxi_lcd_pin_cfg(sel, 0);
	sunxi_lcd_delay_ms(5);
	panel_reset(sel, 0);
	sunxi_lcd_delay_ms(20);
	sunxi_lcd_power_disable(sel, 0); //config lcd_power pin to close lcd power
}

static void lcd_bl_open(u32 sel)
{
	sunxi_lcd_pwm_enable(sel);
	sunxi_lcd_backlight_enable(sel); //config lcd_bl_en pin to open lcd backlight
}

static void lcd_bl_close(u32 sel)
{
	sunxi_lcd_backlight_disable(sel); //config lcd_bl_en pin to close lcd backlight
	sunxi_lcd_pwm_disable(sel);
}

static void lcd_panel_init(u32 sel)
{
	bool TxPLL_unlock = 1;
	s32 ret;
	ret = i2c_add_driver(&lt8911ex_i2c_driver);
	if (ret) {
		pr_warn("Add lt8911ex_i2c_driver fail!\n");
	}
	lt8911ex_chipID_read();
	lt8911ex_eDP_video_cfg();
	lt8911ex_init();
	lt8911ex_read_edid(); //for debug
	Read_DPCD010A = lt8911ex_Dpcd_read(0x010A) & 0x01;
	DEBUG("%s (%u) Read_DPCD010A: 0x%x\n", __FUNCTION__, __LINE__, Read_DPCD010A);
	if (Read_DPCD010A)
		ScrambleMode = 1; //eDP scramble
	else
		ScrambleMode = 0; //DP scramble
	DEBUG("%s (%u) ScrambleMode = %d\n", __FUNCTION__, __LINE__, ScrambleMode);
#ifndef _EDP_Pattern_
	do {
			i2c_smbus_write_byte_data(this_client, 0xff, 0x85);
			if ((i2c_smbus_read_byte_data(this_client, 0x12) & 0xc0) == 0x40) { //clock stable
				i2c_smbus_write_byte_data(this_client, 0xff, 0x87);
				if ((i2c_smbus_read_byte_data(this_client, 0x37) & 0x04) == 0x00) {  //pll unlocked
					i2c_smbus_write_byte_data(this_client, 0xff, 0x81);
					i2c_smbus_write_byte_data(this_client, 0x04, 0xfd);
					sunxi_lcd_delay_ms(50);
					i2c_smbus_write_byte_data(this_client, 0x04, 0xff);
				} else
					DEBUG("%s (%u) lvds rxpll unlocked, reset rxpll!\n", __FUNCTION__, __LINE__);

				if ((i2c_smbus_read_byte_data(this_client, 0x37) & 0x02) == 0x00) { //txpll unlocked
					lt8911ex_reset(sel);
					lt8911ex_chipID_read();
					lt8911ex_eDP_video_cfg();
					lt8911ex_init();
					TxPLL_unlock = 1;
				} else
					TxPLL_unlock = 0;
			}
			sunxi_lcd_delay_ms(200);
			lt8911ex_lvds_clock_check();
			lt8911ex_lvds_video_check();

			if ((h_active != LVDS_Timing[hact]) || (v_active != LVDS_Timing[vact])) {
				i2c_smbus_write_byte_data(this_client, 0xff, 0x81); //register bank
				i2c_smbus_write_byte_data(this_client, 0x05, 0xae);
				sunxi_lcd_delay_ms(10);
				i2c_smbus_write_byte_data(this_client, 0x05, 0xfe); //reset lvds digtal
				DEBUG("%s (%u) No LVDS Signal Input!\n", __FUNCTION__, __LINE__);
			}
	} while (TxPLL_unlock || (h_active != LVDS_Timing[hact]) || (v_active != LVDS_Timing[vact])); //Wait for LVDS signal input
	DEBUG("%s (%u) TxPLL_unlock = %d, h_active = %d, v_active = %d\n", __FUNCTION__, __LINE__, \
			TxPLL_unlock, h_active, v_active);
	lt8911ex_eDP_video_cfg();
	lt8911ex_init();
#endif
	i2c_smbus_write_byte_data(this_client, 0xff, 0x81);
	i2c_smbus_write_byte_data(this_client, 0x06, 0xdf);
	sunxi_lcd_delay_ms(10);
	i2c_smbus_write_byte_data(this_client, 0x06, 0xff);

	lt8911ex_link_train();
	lt8911ex_txswing_preset();

	//just for debug
	lt8911ex_link_train_result();
}

static void lcd_panel_exit(u32 sel)
{
	if (this_client) {
		i2c_del_driver(&lt8911ex_i2c_driver);
		this_client = NULL;
	}
}

static s32 lcd_user_defined_func(u32 sel, u32 para1, u32 para2, u32 para3)
{
	return 0;
}

struct __lcd_panel lt8911ex_lvds_edp_panel = {
	.name = "lt8911ex",
	.func = {
		.cfg_panel_info = lcd_cfg_panel_info,
		.cfg_open_flow = lcd_open_flow,
		.cfg_close_flow = lcd_close_flow,
		.lcd_user_defined_func = lcd_user_defined_func,
	},
};

