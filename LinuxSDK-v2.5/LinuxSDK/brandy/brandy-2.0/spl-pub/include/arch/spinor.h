/*
 * (C) Copyright 20018-2019
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * Description: spinor driver for General spinor operations
 * Author: wangwei <wangwei@allwinnertech.com>
 * Date: 2018-11-15 14:18:18
 */

#ifndef __SUNXI_SPINOR_H
#define __SUNXI_SPINOR_H

#include <common.h>
#include <asm/io.h>


/* Erase commands */
#define CMD_ERASE_4K			0x20
#define CMD_ERASE_CHIP			0xc7
#define CMD_ERASE_64K			0xd8

/* Write commands */
#define CMD_WRITE_STATUS		0x01
#define CMD_PAGE_PROGRAM		0x02
#define CMD_WRITE_DISABLE		0x04
#define CMD_WRITE_ENABLE		0x06
#define CMD_QUAD_PAGE_PROGRAM		0x32

/* Read commands */
#define CMD_READ_ARRAY_SLOW		0x03
#define CMD_READ_ARRAY_FAST		0x0b
#define CMD_READ_DUAL_OUTPUT_FAST	0x3b
#define CMD_READ_DUAL_IO_FAST		0xbb
#define CMD_READ_QUAD_OUTPUT_FAST	0x6b
#define CMD_READ_QUAD_IO_FAST		0xeb
#define CMD_READ_ID			0x9f
#define CMD_READ_STATUS			0x05
#define CMD_READ_STATUS1		0x35
#define CMD_READ_CONFIG			0x35
#define CMD_FLAG_STATUS			0x70



#define SPINOR_OP_READ_1_1_2    0x3b    /* Read data bytes (Dual SPI) */
#define SPINOR_OP_READ_1_1_4    0x6b    /* Read data bytes (Quad SPI) */
/* 4-byte address opcodes - used on Spansion and some Macronix flashes. */
#define SPINOR_OP_READ4_1_1_2   0x3c    /* Read data bytes (Dual SPI) */
#define SPINOR_OP_READ4_1_1_4   0x6c    /* Read data bytes (Quad SPI) */

/* CFI Manufacture ID's */
#define SPI_FLASH_CFI_MFR_SPANSION      0x01
#define SPI_FLASH_CFI_MFR_STMICRO       0x20
#define SPI_FLASH_CFI_MFR_MACRONIX      0xc2
#define SPI_FLASH_CFI_MFR_SST           0xbf
#define SPI_FLASH_CFI_MFR_WINBOND       0xef
#define SPI_FLASH_CFI_MFR_ATMEL         0x1f

#define STATUS_WIP				(1<<0)
#define STATUS_QEB_WINSPAN		(1<<1)
#define STATUS_QEB_MXIC			(1<<6)



int spinor_init(int stage);
int spinor_exit(int force);
int spinor_read(uint start, uint nblock, void *buffer);


#endif
