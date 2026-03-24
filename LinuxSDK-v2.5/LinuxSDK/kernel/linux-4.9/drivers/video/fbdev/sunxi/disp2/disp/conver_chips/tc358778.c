/* drivers/video/fbdev/sunxi/disp2/disp/conver_chips/tc358778.c
*
* Copyright (c) 2021 Allwinnertech Co., Ltd.
* Author: hongyaobin <hongyaobin@allwinnertech.com>
*
* RGB888 to MIPI (he0801a068 panel)driver
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*/

#include "tc358778.h"

static struct i2c_client *this_client;

extern s32 bsp_disp_get_panel_info(u32 screen_id, struct disp_panel_para *info);
extern s32 bsp_disp_get_output_type(u32 disp);

static void lcd_power_on(u32 sel);
static void lcd_power_off(u32 sel);
static void lcd_bl_open(u32 sel);
static void lcd_bl_close(u32 sel);

static void lcd_panel_init(u32 sel);
static void lcd_panel_exit(u32 sel);

#define tc358778_RST	0
#define TC358778_SLAVE_ADDR 0x0e
#define TC358778_I2C_ID	0x4 //TWI4

#define DCS_LONG_WRITE 	0x4039
#define DCS_SHORT_WRITE 0x1015
#define DCS_NOP_WRITE 	0x1005

#define panel_reset(sel, val) sunxi_lcd_gpio_set_value(sel, tc358778_RST, val)

#define REGFLAG_DELAY			0XFE
#define REGFLAG_END_OF_TABLE	0xFF	//END OF REGISTERS MARKER

struct LCM_setting_table {
	u8 cmd;
	u32 count;
	u8 para_list[80];
};

static struct LCM_setting_table LCM_HE0801A068_setting[] = {
	/*========== Internal setting ==========*/
	{ 0xFF, 0x4, { 0xAA, 0x55, 0xA5, 0x80 } },
	{ 0x6F, 0x2, { 0x11, 0x00 } },
	{ 0xF7, 0x2, { 0x20, 0x00 } },
	{ 0x6F, 0x1, { 0x06 } },
	{ 0xF7, 0x1, { 0xA0 } },
	{ 0x6F, 0x1, { 0x19 } },
	{ 0xF7, 0x1, { 0x12 } },
	{ 0xF4, 0x1, { 0x03 } },
	{ 0x6F, 0x1, { 0x08 } },
	{ 0xFA, 0x1, { 0x40 } },
	{ 0x6F, 0x1, { 0x11 } },
	{ 0xF3, 0x1, { 0x01 } },
	/*========== page0 relative ==========*/
	{ 0xF0, 0x5, { 0x55, 0xAA, 0x52, 0x08, 0x00 } },
	{ 0xC8, 0x1, { 0x80 } },
	{ 0xB1, 0x2, { 0x6C, 0x07 } },
	{ 0xB6, 0x1, { 0x08 } },
	{ 0x6F, 0x1, { 0x02 } },
	{ 0xB8, 0x1, { 0x08 } },
	{ 0xBB, 0x2, { 0x74, 0x44 } },
	{ 0xBC, 0x2, { 0x00, 0x00 } },
	{ 0xBD, 0x5, { 0x02, 0xB0, 0x0C, 0x0A, 0x00 } },
	/*========== page1 relative ==========*/
	{ 0xF0, 0x5, { 0x55, 0xAA, 0x52, 0x08, 0x01 } },
	{ 0xB0, 0x2, { 0x05, 0x05 } },
	{ 0xB1, 0x2, { 0x05, 0x05 } },
	{ 0xBC, 0x2, { 0x90, 0x01 } },
	{ 0xBD, 0x2, { 0x90, 0x01 } },
	{ 0xCA, 0x1, { 0x00 } },
	{ 0xC0, 0x1, { 0x04 } },
	{ 0xBE, 0x1, { 0x29 } },
	{ 0xB3, 0x2, { 0x37, 0x37 } },
	{ 0xB4, 0x2, { 0x19, 0x19 } },
	{ 0xB9, 0x2, { 0x44, 0x44 } },
	{ 0xBA, 0x2, { 0x24, 0x24 } },
	/*========== page2 relative ==========*/
	{ 0xF0, 0x5, { 0x55, 0xAA, 0x52, 0x08, 0x02 } },
	{ 0xEE, 0x1, { 0x01 } },
	{ 0xEF, 0x4, { 0x09, 0x06, 0x15, 0x18 } },
	{ 0xB0, 0x6, { 0x00, 0x00, 0x00, 0x25, 0x00, 0x43 } },
	{ 0x6F, 0x1, { 0x06 } },
	{ 0xB0, 0x6, { 0x00, 0x54, 0x00, 0x68, 0x00, 0xA0 } },
	{ 0x6F, 0x1, { 0x0C } },
	{ 0xB0, 0x4, { 0x00, 0xC0, 0x01, 0x00 } },
	{ 0xB1, 0x6, { 0x01, 0x30, 0x01, 0x78, 0x01, 0xAE } },
	{ 0x6F, 0x1, { 0x06 } },
	{ 0xB1, 0x6, { 0x02, 0x08, 0x02, 0x52, 0x02, 0x54 } },
	{ 0x6F, 0x1, { 0x0C } },
	{ 0xB1, 0x4, { 0x02, 0x99, 0x02, 0xF0 } },
	{ 0xB2, 0x6, { 0x03, 0x20, 0x03, 0x56, 0x03, 0x76 } },
	{ 0x6F, 0x1, { 0x06 } },
	{ 0xB2, 0x6, { 0x03, 0x93, 0x03, 0xA4, 0x03, 0xB9 } },
	{ 0x6F, 0x1, { 0x0C } },
	{ 0xB2, 0x4, { 0x03, 0xC9, 0x03, 0xE3 } },
	{ 0xB3, 0x4, { 0x03, 0xFC, 0x03, 0xFF } },
	/*========== GOA relative ==========*/
	{ 0xF0, 0x5, { 0x55, 0xAA, 0x52, 0x08, 0x06 } },
	{ 0xB0, 0x2, { 0x00, 0x10 } },
	{ 0xB1, 0x2, { 0x12, 0x14 } },
	{ 0xB2, 0x2, { 0x16, 0x18 } },
	{ 0xB3, 0x2, { 0x1A, 0x29 } },
	{ 0xB4, 0x2, { 0x2A, 0x08 } },
	{ 0xB5, 0x2, { 0x31, 0x31 } },
	{ 0xB6, 0x2, { 0x31, 0x31 } },
	{ 0xB7, 0x2, { 0x31, 0x31 } },
	{ 0xB8, 0x2, { 0x31, 0x0A } },
	{ 0xB9, 0x2, { 0x31, 0x31 } },
	{ 0xBA, 0x2, { 0x31, 0x31 } },
	{ 0xBB, 0x2, { 0x0B, 0x31 } },
	{ 0xBC, 0x2, { 0x31, 0x31 } },
	{ 0xBD, 0x2, { 0x31, 0x31 } },
	{ 0xBE, 0x2, { 0x31, 0x31 } },
	{ 0xBF, 0x2, { 0x09, 0x2A } },
	{ 0xC0, 0x2, { 0x29, 0x1B } },
	{ 0xC1, 0x2, { 0x19, 0x17 } },
	{ 0xC2, 0x2, { 0x15, 0x13 } },
	{ 0xC3, 0x2, { 0x11, 0x01 } },
	{ 0xE5, 0x2, { 0x31, 0x31 } },
	{ 0xC4, 0x2, { 0x09, 0x1B } },
	{ 0xC5, 0x2, { 0x19, 0x17 } },
	{ 0xC6, 0x2, { 0x15, 0x13 } },
	{ 0xC7, 0x2, { 0x11, 0x29 } },
	{ 0xC8, 0x2, { 0x2A, 0x01 } },
	{ 0xC9, 0x2, { 0x31, 0x31 } },
	{ 0xCA, 0x2, { 0x31, 0x31 } },
	{ 0xCB, 0x2, { 0x31, 0x31 } },
	{ 0xCC, 0x2, { 0x31, 0x0B } },
	{ 0xCD, 0x2, { 0x31, 0x31 } },
	{ 0xCE, 0x2, { 0x31, 0x31 } },
	{ 0xCF, 0x2, { 0x0A, 0x31 } },
	{ 0xD0, 0x2, { 0x31, 0x31 } },
	{ 0xD1, 0x2, { 0x31, 0x31 } },
	{ 0xD2, 0x2, { 0x31, 0x31 } },
	{ 0xD3, 0x2, { 0x00, 0x2A } },
	{ 0xD4, 0x2, { 0x29, 0x10 } },
	{ 0xD5, 0x2, { 0x12, 0x14 } },
	{ 0xD6, 0x2, { 0x16, 0x18 } },
	{ 0xD7, 0x2, { 0x1A, 0x08 } },
	{ 0xE6, 0x2, { 0x31, 0x31 } },
	{ 0xD8, 0x5, { 0x00, 0x00, 0x00, 0x54, 0x00 } },
	{ 0xD9, 0x5, { 0x00, 0x15, 0x00, 0x00, 0x00 } },
	{ 0xE7, 0x1, { 0x00 } },
	/*PAGE3 :*/
	{ 0xF0, 0x5, { 0x55, 0xAA, 0x52, 0x08, 0x03 } },
	{ 0xB0, 0x2, { 0x20, 0x00 } },
	{ 0xB1, 0x2, { 0x20, 0x00 } },
	{ 0xB2, 0x5, { 0x05, 0x00, 0x00, 0x00, 0x00 } },
	{ 0xB6, 0x5, { 0x05, 0x00, 0x00, 0x00, 0x00 } },
	{ 0xB7, 0x5, { 0x05, 0x00, 0x00, 0x00, 0x00 } },
	{ 0xBA, 0x5, { 0x57, 0x00, 0x00, 0x00, 0x00 } },
	{ 0xBB, 0x5, { 0x57, 0x00, 0x00, 0x00, 0x00 } },
	{ 0xC0, 0x4, { 0x00, 0x00, 0x00, 0x00 } },
	{ 0xC1, 0x4, { 0x00, 0x00, 0x00, 0x00 } },
	{ 0xC4, 0x1, { 0x60 } },
	{ 0xC5, 0x1, { 0x40 } },
	/*PAGE5 :*/
	{ 0xF0, 0x5, { 0x55, 0xAA, 0x52, 0x08, 0x05 } },
	{ 0xBD, 0x5, { 0x03, 0x01, 0x03, 0x03, 0x03 } },
	{ 0xB0, 0x2, { 0x17, 0x06 } },
	{ 0xB1, 0x2, { 0x17, 0x06 } },
	{ 0xB2, 0x2, { 0x17, 0x06 } },
	{ 0xB3, 0x2, { 0x17, 0x06 } },
	{ 0xB4, 0x2, { 0x17, 0x06 } },
	{ 0xB5, 0x2, { 0x17, 0x06 } },
	{ 0xB8, 0x1, { 0x00 } },
	{ 0xB9, 0x1, { 0x00 } },
	{ 0xBA, 0x1, { 0x00 } },
	{ 0xBB, 0x1, { 0x02 } },
	{ 0xBC, 0x1, { 0x00 } },
	{ 0xC0, 0x1, { 0x07 } },
	{ 0xC4, 0x1, { 0x80 } },
	{ 0xC5, 0x1, { 0xA4 } },
	{ 0xC8, 0x2, { 0x05, 0x30 } },
	{ 0xC9, 0x2, { 0x01, 0x31 } },
	{ 0xCC, 0x3, { 0x00, 0x00, 0x3C } },
	{ 0xCD, 0x3, { 0x00, 0x00, 0x3C } },
	{ 0xD1, 0x5, { 0x00, 0x04, 0xFD, 0x07, 0x10 } },
	{ 0xD2, 0x5, { 0x00, 0x05, 0x02, 0x07, 0x10 } },
	{ 0xE5, 0x1, { 0x06 } },
	{ 0xE6, 0x1, { 0x06 } },
	{ 0xE7, 0x1, { 0x06 } },
	{ 0xE8, 0x1, { 0x06 } },
	{ 0xE9, 0x1, { 0x06 } },
	{ 0xEA, 0x1, { 0x06 } },
	{ 0xED, 0x1, { 0x30 } },
	{ 0x6F, 0x1, { 0x11 } },
	/*reload setting*/
	{ 0xF3, 0x1, { 0x01 } },
	{ 0x35, 0x0, { 0x00 } },
	{ 0x11, 0x0, { 0x00 } },
	{ REGFLAG_DELAY, REGFLAG_DELAY, { 80 } },
	{ 0x29, 0x0, { 0x00 } },
	{ REGFLAG_END_OF_TABLE, REGFLAG_END_OF_TABLE, {} }
};

static const struct of_device_id tc358778_match[] = {
	{.compatible = "allwinner,sunxi-disp",},
	{},
};

static  unsigned short normal_i2c[] = {TC358778_SLAVE_ADDR, I2C_CLIENT_END};

typedef struct tc358778_reg {
	u8 offset;
	u8 mask;
	u8 value;
} tc358778_reg_set;

static const struct i2c_device_id tc358778_id[] = {
	{"TC358778", TC358778_I2C_ID}, /* name, private_data */
	{/* End of list */}
};
MODULE_DEVICE_TABLE(i2c, tc358778_id);

static int tc358778_twi_write_reg(u16 reg_addr, u8 length, u32 value)
{
	u8 data[6];
	data[0] = (reg_addr & 0xff00) >> 8; //MSB first
	data[1] = (reg_addr & 0x00ff);
	if (length == 2) { //the tc358778 has both 16bits and 32 bits registers
		data[2] = (value & 0xff00) >> 8;
		data[3] = (value & 0x00ff);
		data[4] = 0;
		data[5] = 0;
	} else if (length == 4) {
		data[2] = (value & 0xff000000) >> 24;
		data[3] = (value & 0x00ff0000) >> 16;
		data[4] = (value & 0x0000ff00) >> 8;
		data[5] = (value & 0x000000ff);
	} else {
		printk("[DEBUG_MSG] %s (%u) invalid length!\n", __FUNCTION__, __LINE__);
		return -1;
	}
	if (i2c_master_send(this_client, data, (length + 2)) == (length + 2)) {
		//printk("[DEBUG_MSG] %s (%u) send ok\n", __FUNCTION__, __LINE__);
		return 0;
	} else {
		//printk("[DEBUG_MSG] %s (%u) send failed\n", __FUNCTION__, __LINE__);
		return -EIO;
	}
}

#if 0
static int tc358778_twi_read_reg(u16 reg_addr, u8 length, u8 *data)
{
	int ret;
	struct i2c_msg tc358778_read_msgs[2];
	u8 addr[2];
	addr[0] = (reg_addr & 0xff00) >> 8;
	addr[1] = reg_addr & 0x00ff;
	/* write the register address */
	tc358778_read_msgs[0].addr = this_client->addr;
	tc358778_read_msgs[0].flags = 0;
	tc358778_read_msgs[0].len = 2; //the tc358778's register address is 16 bits
	tc358778_read_msgs[0].buf = addr;
	/* read data */
	tc358778_read_msgs[1].addr = this_client->addr;
	tc358778_read_msgs[1].flags = I2C_M_RD;
	tc358778_read_msgs[1].len = length;
	tc358778_read_msgs[1].buf = data;

	ret = i2c_transfer(this_client->adapter, tc358778_read_msgs, 2);
	if (ret != 2) {
		dev_err(&this_client->dev, "i2c_transfer() returned %d\n", ret);
		return -1;
	} else
		return ret;
}

static int tc358778_dcs_write_0para(u8 cmd)
{
	int ret;
	u16 temp_data = 0x0000;
	temp_data |= cmd;
	ret = tc358778_twi_write_reg(0x0602, 2, DCS_NOP_WRITE);
	ret += tc358778_twi_write_reg(0x0604, 2, 0X0000);
	ret += tc358778_twi_write_reg(0x0610, 2, temp_data);
	ret += tc358778_twi_write_reg(0x0600, 2, 0X0001); //Start DCS Command transfer
	sunxi_lcd_delay_us(20000);
	if (ret)
		return -EIO;
	else
		return 0;
}

static int tc358778_dcs_write_1para(u8 cmd, u8 param)
{
	int ret;
	u16 temp_data = ((param << 8) & 0xff00) | cmd;
	ret = tc358778_twi_write_reg(0x0602, 2, DCS_SHORT_WRITE);
	ret += tc358778_twi_write_reg(0x0604, 2, 0X0000);
	ret += tc358778_twi_write_reg(0x0610, 2, temp_data);
	ret += tc358778_twi_write_reg(0x0600, 2, 0X0001); //Start DCS Command transfer
	sunxi_lcd_delay_us(200);
	if (ret)
		return -EIO;
	else
		return 0;
}
#endif

static int tc358778_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	this_client = client;
	return 0;
}

static int tc358778_i2c_remove(struct i2c_client *client)
{
	return 0;
}

static int tc358778_i2c_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	const char *type_name = "TC358778";
	printk("[DEBUG_MSG] %s (%u) tc358778_i2c_detect\n", __FUNCTION__, __LINE__);
	if (TC358778_I2C_ID == client->adapter->nr) {
		strlcpy(info->type, type_name, 20);
	} else
		pr_warn("%s:%d wrong i2c id:%d, expect id is :%d\n", __func__, __LINE__,
			client->adapter->nr, TC358778_I2C_ID);
	return 0;
}


static struct i2c_driver tc358778_i2c_driver = {
	.class = I2C_CLASS_HWMON,
	.id_table = tc358778_id,
	.probe = tc358778_i2c_probe,
	.remove = tc358778_i2c_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "TC358778",
		.of_match_table = tc358778_match,
	},
	.detect = tc358778_i2c_detect,
	.address_list = normal_i2c,
};

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
	LCD_OPEN_FUNC(sel, sunxi_lcd_tcon_enable, 10);
	LCD_OPEN_FUNC(sel, lcd_panel_init, 10);
	LCD_OPEN_FUNC(sel, lcd_bl_open, 0);

	return 0;
}

static s32 lcd_close_flow(u32 sel)
{
	LCD_CLOSE_FUNC(sel, lcd_bl_close, 0);
	LCD_CLOSE_FUNC(sel, sunxi_lcd_tcon_disable, 0);
	LCD_CLOSE_FUNC(sel, lcd_panel_exit,	200);
	LCD_CLOSE_FUNC(sel, lcd_power_off, 500);

	return 0;
}

static void lcd_power_on(u32 sel)
{
    /* lcd设备节点下自定义IO的电平控制函数：0：对应 IO 输出低电平 1：对应 IO 输出高电平 */
	panel_reset(sel, 0); //lcd_gpio_0

	/* 打开/关闭 lcd 电源，操作的是board.dts中的lcd_power/lcd_power1/lcd_power2 */
	sunxi_lcd_power_enable(sel, 0);
	sunxi_lcd_delay_ms(5);
	sunxi_lcd_power_enable(sel, 1);
	sunxi_lcd_delay_ms(5);
	sunxi_lcd_pin_cfg(sel, 1); //配置lcd相关的gpio
	sunxi_lcd_delay_ms(10);
	panel_reset(sel, 1);
	sunxi_lcd_delay_ms(10);
	panel_reset(sel, 0);
	sunxi_lcd_delay_ms(20);
	panel_reset(sel, 1);
	sunxi_lcd_delay_ms(20);
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

static void he0801a068_init(void)
{
	int i = 0;
	for (i = 0; ; i++) {
		u16 j = 0, temp = 0, reg_addr = 0, cnt = 0;
		if (LCM_HE0801A068_setting[i].count == REGFLAG_END_OF_TABLE)
			break;
		else if (LCM_HE0801A068_setting[i].count == REGFLAG_DELAY) {
			sunxi_lcd_delay_ms(LCM_HE0801A068_setting[i].para_list[0]);
		} else {
			if (LCM_HE0801A068_setting[i].count > 1) {
				if ((LCM_HE0801A068_setting[i].count + 1) % 2 == 0) //include the command byte
					cnt = (LCM_HE0801A068_setting[i].count + 1) >> 1;
				else
					cnt = ((LCM_HE0801A068_setting[i].count + 1) >> 1) + 1;
				tc358778_twi_write_reg(0x0008, 2, 0x0001);
				tc358778_twi_write_reg(0x0050, 2, 0x0039);
				tc358778_twi_write_reg(0x0022, 2, LCM_HE0801A068_setting[i].count + 1);
				tc358778_twi_write_reg(0x00E0, 2, 0x8000); //Enable Write into video buffer via I2C/SPI bus
				for (j = 0; j < cnt; j++) {
					reg_addr = 0x00E8;
					temp = 0;
					if (j == 0) {
						temp = ((LCM_HE0801A068_setting[i].para_list[j] << 8) & 0xFF00);
						temp |= LCM_HE0801A068_setting[i].cmd;
					} else if (j * 2 == LCM_HE0801A068_setting[i].count) {
						temp = (LCM_HE0801A068_setting[i].para_list[j * 2 - 1]);
					} else {
						temp = ((LCM_HE0801A068_setting[i].para_list[j * 2] << 8) & 0xFF00);
						temp |= (LCM_HE0801A068_setting[i].para_list[j * 2 - 1]);
					}
					tc358778_twi_write_reg(reg_addr, 2, temp);
				}
				if ((LCM_HE0801A068_setting[i].count + 1) % 4 != 0) //4 bytes algined
					tc358778_twi_write_reg(0x00E8, 2, 0x0000);
				tc358778_twi_write_reg(0x00E0, 2, 0xE000); //Start DSI Tx command transfer
				sunxi_lcd_delay_ms(1);
				tc358778_twi_write_reg(0x00E0, 2, 0x2000); //Keep Mask High to prevent short packets send out
				tc358778_twi_write_reg(0x00E0, 2, 0x0000); //Stop DSI Tx command transfer
				sunxi_lcd_delay_us(20);
			} else if (LCM_HE0801A068_setting[i].count == 0) {
				temp = LCM_HE0801A068_setting[i].cmd;
				tc358778_twi_write_reg(0x0602, 2, DCS_NOP_WRITE);
				tc358778_twi_write_reg(0x0604, 2, 0x0000);
				tc358778_twi_write_reg(0x0610, 2, temp);
				tc358778_twi_write_reg(0x0600, 2, 0x0001);
				sunxi_lcd_delay_ms(1);
			} else {
				temp = ((LCM_HE0801A068_setting[i].para_list[0] << 8) & 0xff00) | LCM_HE0801A068_setting[i].cmd;
				tc358778_twi_write_reg(0x0602, 2, DCS_SHORT_WRITE);
				tc358778_twi_write_reg(0x0604, 2, 0x0000);
				tc358778_twi_write_reg(0x0610, 2, temp);
				tc358778_twi_write_reg(0x0600, 2, 0x0001);
				sunxi_lcd_delay_ms(1);
			}
		}
	}
}

static void lcd_panel_init(u32 sel)
{
	int ret;
	ret = i2c_add_driver(&tc358778_i2c_driver);
	if (ret) {
		pr_warn("Add tc358778_i2c_driver fail!\n");
	}
	/************************************************************
			TC358778 Software Reset
	************************************************************/
	/*ret = tc358778_twi_read_reg(0x0000, 2, val);
	printk("[DEBUG_MSG] %s (%u) ret = %d, buf[0] = 0x%x, buf[1] = 0x%x\n", __FUNCTION__, __LINE__, ret, val[0], val[1]);*/
	tc358778_twi_write_reg(0X0002, 2, 0X0001); //software reset
	sunxi_lcd_delay_ms(1);
	tc358778_twi_write_reg(0X0002, 2, 0X0000); //software reset release
	sunxi_lcd_delay_ms(1);
	/************************************************************
			TC358778 PLL,Clock Setting
	************************************************************/
	tc358778_twi_write_reg(0X0016, 2, 0X208b);
	tc358778_twi_write_reg(0x0018, 2, 0x0603);
	sunxi_lcd_delay_ms(1);
	tc358778_twi_write_reg(0x0018, 2, 0x0613);
	/************************************************************
			TC358778 DPI Input Control
	************************************************************/
	tc358778_twi_write_reg(0x0006, 2, 0x001e);
	/************************************************************
			TC358778 D-PHY Setting
	************************************************************/
	tc358778_twi_write_reg(0x0140, 2, 0x0000); //D-PHY Clock lane enable
	tc358778_twi_write_reg(0x0142, 2, 0x0000);
	tc358778_twi_write_reg(0x0144, 2, 0x0000); //D-PHY Data lane0 enable
	tc358778_twi_write_reg(0x0146, 2, 0x0000);
	tc358778_twi_write_reg(0x0148, 2, 0x0000); //D-PHY Data lane1 enable
	tc358778_twi_write_reg(0x014a, 2, 0x0000);
	tc358778_twi_write_reg(0x014c, 2, 0x0000); //D-PHY Data lane2 enable
	tc358778_twi_write_reg(0x014e, 2, 0x0000);
	tc358778_twi_write_reg(0x0150, 2, 0x0000); //D-PHY Data lane3 enable
	tc358778_twi_write_reg(0x0152, 2, 0x0000);
	tc358778_twi_write_reg(0x0100, 2, 0x0002); //D-PHY Clock lane control
	tc358778_twi_write_reg(0x0102, 2, 0x0000);
	tc358778_twi_write_reg(0x0104, 2, 0x0002); //D-PHY Data lane0 control
	tc358778_twi_write_reg(0x0106, 2, 0x0000);
	tc358778_twi_write_reg(0x0108, 2, 0x0002); //D-PHY Data lane1 control
	tc358778_twi_write_reg(0x010a, 2, 0x0000);
	tc358778_twi_write_reg(0x010c, 2, 0x0002); //D-PHY Data lane2 control
	tc358778_twi_write_reg(0x010e, 2, 0x0000);
	tc358778_twi_write_reg(0x0110, 2, 0x0002); //D-PHY Data lane3 control
	tc358778_twi_write_reg(0x0112, 2, 0x0000);
	/************************************************************
			TC358778 DSI-TX PPI Control
	************************************************************/
	tc358778_twi_write_reg(0x0210, 2, 0x1644); //LINEINITCNT
	tc358778_twi_write_reg(0x0212, 2, 0x0000);
	tc358778_twi_write_reg(0x0214, 2, 0x0004); //LPTXTIMECNT
	tc358778_twi_write_reg(0x0216, 2, 0x0000);
	tc358778_twi_write_reg(0x0218, 2, 0x2002); //TCLK_HEADERCNT
	tc358778_twi_write_reg(0x021a, 2, 0x0000);
	tc358778_twi_write_reg(0x021c, 2, 0x0003); //TCLK_TRAILCNT
	tc358778_twi_write_reg(0x021e, 2, 0x0000);
	tc358778_twi_write_reg(0x0220, 2, 0x0603); //THS_HEADERCNT
	tc358778_twi_write_reg(0x0222, 2, 0x0000);
	tc358778_twi_write_reg(0x0224, 2, 0x4650); //TWAKEUPCNT
	tc358778_twi_write_reg(0x0226, 2, 0x0000);
	tc358778_twi_write_reg(0x0228, 2, 0x000B); //TCLK_POSTCNT
	tc358778_twi_write_reg(0x022a, 2, 0x0000);
	tc358778_twi_write_reg(0x022c, 2, 0x0001); //THS_TRAILCNT
	tc358778_twi_write_reg(0x022e, 2, 0x0000);
	tc358778_twi_write_reg(0x0230, 2, 0x0005); //HSTXVREGCNT
	tc358778_twi_write_reg(0x0232, 2, 0x0000);
	tc358778_twi_write_reg(0x0234, 2, 0x001F); //HSTXVREGEN enable
	tc358778_twi_write_reg(0x0236, 2, 0x0000);
	tc358778_twi_write_reg(0x0238, 2, 0x0001); //DSI clock Enable/Disable during LP
	tc358778_twi_write_reg(0x023a, 2, 0x0000);
	tc358778_twi_write_reg(0x023c, 2, 0x0005); //BTACNTRL1
	tc358778_twi_write_reg(0x023e, 2, 0x0004);
	tc358778_twi_write_reg(0x0204, 2, 0x0001); //STARTCNTRL
	tc358778_twi_write_reg(0x0206, 2, 0x0000);
	/************************************************************
			TC358778 DSI-TX Timing Control
	************************************************************/
	tc358778_twi_write_reg(0x0620, 2, 0x0001); //Sync Pulse/Sync Event mode setting
	tc358778_twi_write_reg(0x0622, 2, 0x000d); //V Control Register1
	tc358778_twi_write_reg(0x0624, 2, 0x0008); //V Control Register2
	tc358778_twi_write_reg(0x0626, 2, 0x0500); //V Control Register3
	tc358778_twi_write_reg(0x0628, 2, 0x0127); //H Control Register1
	tc358778_twi_write_reg(0x062A, 2, 0x010c); //H Control Register2
	tc358778_twi_write_reg(0x062C, 2, 0x0960); //H Control Register3
	tc358778_twi_write_reg(0x0518, 2, 0x0001); //DSI Start
	tc358778_twi_write_reg(0x051a, 2, 0x0000);
	/************************************************************
			LCDD (Peripheral) Setting
	************************************************************/
	he0801a068_init();
	/************************************************************
			Set to HS mode
	************************************************************/
	tc358778_twi_write_reg(0x0500, 2, 0x0086); //DSI lane setting, DSI mode=HS
	tc358778_twi_write_reg(0x0502, 2, 0xa300);
	tc358778_twi_write_reg(0x0500, 2, 0x8000); //Switch to DSI mode
	tc358778_twi_write_reg(0x0502, 2, 0xc300);
	/************************************************************
			Host: RGB(DPI) input start
	************************************************************/
	tc358778_twi_write_reg(0x0008, 2, 0x0037); //DSI-TX Format setting
	tc358778_twi_write_reg(0x0050, 2, 0x003E); //DSI-TX Pixel stream packet Data Type setting
	tc358778_twi_write_reg(0x0032, 2, 0x0000); //HSYNC Polarity
	tc358778_twi_write_reg(0x0004, 2, 0x0044); //Configuration Control tc358778_twi_write_reg
}

static void lcd_panel_exit(u32 sel)
{
	if (this_client) {
		i2c_del_driver(&tc358778_i2c_driver);
		this_client = NULL;
	}
}

static s32 lcd_user_defined_func(u32 sel, u32 para1, u32 para2, u32 para3)
{
	return 0;
}

struct __lcd_panel tc358778_rgb_mipi_panel = {
	.name = "tc358778",
	.func = {
		.cfg_panel_info = lcd_cfg_panel_info,
		.cfg_open_flow = lcd_open_flow,
		.cfg_close_flow = lcd_close_flow,
		.lcd_user_defined_func = lcd_user_defined_func,
	},
};

