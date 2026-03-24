/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012-2012 Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * Configuration settings for the Allwinner sunxi series of boards.
 */

#ifndef _SUNXI_COMMON_CONFIG_H
#define _SUNXI_COMMON_CONFIG_H

#include <asm/arch/cpu.h>
#include <linux/stringify.h>

#define MIN(x,y) (x < y ? x : y)

#define SUNXI_STACKSIZE_IRQ    (4*1024)        /* IRQ stack */
#define SUNXI_STACKSIZE_FIQ    (4*1024)        /* FIQ stack */

#ifdef CONFIG_SUNXI_DEBUG
#define DEBUG
#endif

#ifdef CONFIG_OLD_SUNXI_KERNEL_COMPAT
/*
 * The U-Boot workarounds bugs in the outdated buggy sunxi-3.4 kernels at the
 * expense of restricting some features, so the regular machine id values can
 * be used.
 */
# define CONFIG_MACH_TYPE_COMPAT_REV	0
#else
/*
 * A compatibility guard to prevent loading outdated buggy sunxi-3.4 kernels.
 * Only sunxi-3.4 kernels with appropriate fixes applied are able to pass
 * beyond the machine id check.
 */
# define CONFIG_MACH_TYPE_COMPAT_REV	1
#endif

#ifdef CONFIG_ARM64
#define CONFIG_BUILD_TARGET "u-boot.itb"
#define CONFIG_SYS_BOOTM_LEN		(32 << 20)
#endif

/* Serial & console */
#define CONFIG_SYS_NS16550_SERIAL
/* ns16550 reg in the low bits of cpu reg */
#define CONFIG_SYS_NS16550_CLK		24000000
#ifndef CONFIG_DM_SERIAL
# define CONFIG_SYS_NS16550_REG_SIZE	-4
# define CONFIG_SYS_NS16550_COM1		SUNXI_UART0_BASE
# define CONFIG_SYS_NS16550_COM2		SUNXI_UART1_BASE
# define CONFIG_SYS_NS16550_COM3		SUNXI_UART2_BASE
# define CONFIG_SYS_NS16550_COM4		SUNXI_UART3_BASE
/*# define CONFIG_SYS_NS16550_COM5		SUNXI_R_UART_BASE*/
#endif

/* CPU */
#define COUNTER_FREQUENCY		24000000

/*
 * The DRAM Base differs between some models. We cannot use macros for the
 * CONFIG_FOO defines which contain the DRAM base address since they end
 * up unexpanded in include/autoconf.mk .
 *
 * So we have to have this #ifdef #else #endif block for these.
 */
#ifdef CONFIG_MACH_SUN9I
#define SDRAM_OFFSET(x) 0x2##x
#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_LOAD_ADDR		0x22000000 /* default load address */
/* Note SPL_STACK_R_ADDR is set through Kconfig, we include it here 
 * since it needs to fit in with the other values. By also #defining it
 * we get warnings if the Kconfig value mismatches. */
#define CONFIG_SPL_STACK_R_ADDR		0x2fe00000
#define CONFIG_SPL_BSS_START_ADDR	0x2ff80000
#else
#define SDRAM_OFFSET(x) 0x4##x
#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define CONFIG_SYS_LOAD_ADDR		0x42000000 /* default load address */
/* V3s do not have enough memory to place code at 0x4a000000 */
/* Note SPL_STACK_R_ADDR is set through Kconfig, we include it here 
 * since it needs to fit in with the other values. By also #defining it
 * we get warnings if the Kconfig value mismatches. */
#define CONFIG_SPL_STACK_R_ADDR		0x4fe00000
#define CONFIG_SPL_BSS_START_ADDR	0x4ff80000
#define CONFIG_DRAM_PARA_ADDR		SDRAM_OFFSET(0800000)
#endif

/* Provide a default addr */
#ifndef SUNXI_CFG_SBROMSW_BASE
#define SUNXI_CFG_SBROMSW_BASE 0x20000
#endif
#ifndef SUNXI_CFG_TOC1_STORE_IN_DRAM_BASE
#define SUNXI_CFG_TOC1_STORE_IN_DRAM_BASE SDRAM_OFFSET(2e00000)
#endif

#define CONFIG_SPL_BSS_MAX_SIZE		0x00080000 /* 512 KiB */


#define CONFIG_SYS_INIT_RAM_ADDR	SUNXI_SYS_SRAM_BASE
#define CONFIG_SYS_INIT_RAM_SIZE	SUNXI_SYS_SRAM_SIZE

/*#define CONFIG_SUNXI_LOGBUFFER*/
#define SUNXI_DISPLAY_FRAME_BUFFER_ADDR    SDRAM_OFFSET(6400000)
#define SUNXI_DISPLAY_FRAME_BUFFER_SIZE    0x01000000


#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_0			CONFIG_SYS_SDRAM_BASE
#define PHYS_SDRAM_0_SIZE		0x20000000 /* 512 MiB */

#ifdef CONFIG_AHCI
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SUNXI_AHCI
#define CONFIG_SYS_64BIT_LBA
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	1
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
					 CONFIG_SYS_SCSI_MAX_LUN)
#endif

#if 0
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG
#define CONFIG_SERIAL_TAG
#endif

#ifdef CONFIG_NAND_SUNXI
#define CONFIG_SYS_NAND_MAX_ECCPOS 1664
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_SYS_MAX_NAND_DEVICE 8

#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#endif

#ifdef CONFIG_SPL_SPI_SUNXI
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x8000
#endif

/* mmc config */
#ifdef CONFIG_MMC
#define CONFIG_MMC_SUNXI_SLOT		0
#endif


#if  defined (CONFIG_MACH_SUN8IW18) || defined (CONFIG_MACH_SUN8IW19)
/* 20MB of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (20 << 20))
#else
/* 64MB of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (64 << 20))
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_CBSIZE	1024	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE	1024	/* Print Buffer Size */

/* standalone support */
#define CONFIG_STANDALONE_LOAD_ADDR	CONFIG_SYS_LOAD_ADDR

/* FLASH and environment organization */

#define CONFIG_SYS_MONITOR_LEN		(768 << 10)	/* 768 KiB */

#ifndef CONFIG_ARM64		/* AArch64 FEL support is not ready yet */
#define CONFIG_SPL_BOARD_LOAD_IMAGE
#endif

#ifdef CONFIG_SUNXI_HIGH_SRAM
#define CONFIG_SPL_TEXT_BASE		0x10060		/* sram start+header */
#define CONFIG_SPL_MAX_SIZE		0x7fa0		/* 32 KiB */
#ifdef CONFIG_ARM64
/* end of SRAM A2 for now, as SRAM A1 is pretty tight for an ARM64 build */
#define LOW_LEVEL_SRAM_STACK		0x00054000
#else
#define LOW_LEVEL_SRAM_STACK		0x00018000
#endif /* !CONFIG_ARM64 */
#else
#define CONFIG_SPL_TEXT_BASE		0x60		/* sram start+header */
#define CONFIG_SPL_MAX_SIZE		0x5fa0		/* 24KB on sun4i/sun7i */
#define LOW_LEVEL_SRAM_STACK		0x00008000	/* End of sram */
#endif

#define CONFIG_SPL_STACK		LOW_LEVEL_SRAM_STACK

#define CONFIG_SPL_PAD_TO		32768		/* decimal for 'dd' */


/* I2C */

#if defined CONFIG_I2C0_ENABLE || defined CONFIG_I2C1_ENABLE || \
    defined CONFIG_I2C2_ENABLE || defined CONFIG_I2C3_ENABLE || \
    defined CONFIG_I2C4_ENABLE || defined CONFIG_I2C5_ENABLE || \
    defined CONFIG_R_I2C0_ENABLE || defined CONFIG_R_I2C1_ENABLE
/*#define CONFIG_SYS_I2C_MVTWSI*/
/* #define CONFIG_SYS_I2C_SUNXI*/
#ifndef CONFIG_DM_I2C
#define CONFIG_SYS_I2C
/*#define CONFIG_SYS_I2C_SPEED	400000*/
/*#define CONFIG_SYS_I2C_SLAVE		0x7f*/
#endif
#endif


/* PMU */


/* GPIO */
#define CONFIG_SUNXI_GPIO

#ifdef CONFIG_VIDEO_SUNXI
/*
 * The amount of RAM to keep free at the top of RAM when relocating u-boot,
 * to use as framebuffer. This must be a multiple of 4096.
 */
#define CONFIG_SUNXI_MAX_FB_SIZE (16 << 20)

#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_STD_TIMINGS
#define CONFIG_I2C_EDID
#define VIDEO_LINE_LEN (pGD->plnSizeX)

/* allow both serial and cfb console. */
/* stop x86 thinking in cfbconsole from trying to init a pc keyboard */

#endif /* CONFIG_VIDEO_SUNXI */

/* Ethernet support */
#ifdef CONFIG_SUN4I_EMAC
#ifdef CONFIG_SUNXI_GETH
#ifndef CONFIG_CMD_NET
#define CONFIG_CMD_NET
#endif
#define CONFIG_CMD_PING
#define CONFIG_CMD_MII
#define CONFIG_ETHADDR  84:E7:58:52:10:44
#define CONFIG_IPADDR   192.168.10.100
#define CONFIG_SERVERIP 192.168.10.20
#define CONFIG_NETMASK  255.255.255.0
#define CONFIG_GATEWAYIP 192.168.10.1
#endif
#define CONFIG_MII			/* MII PHY management		*/
#endif

#ifdef CONFIG_SUN7I_GMAC
#define CONFIG_MII			/* MII PHY management		*/
#define CONFIG_PHY_REALTEK
#endif

#ifdef CONFIG_USB_EHCI_HCD
/* #define CONFIG_USB_OHCI_NEW */
/* #define CONFIG_USB_OHCI_SUNXI */
/* #define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS 1 */
#endif


#define CONFIG_MISC_INIT_R

#ifndef CONFIG_SPL_BUILD

#ifdef CONFIG_AHCI
#define BOOT_TARGET_DEVICES_SCSI(func) func(SCSI, scsi, 0)
#else
#define BOOT_TARGET_DEVICES_SCSI(func)
#endif

#ifdef CONFIG_USB_STORAGE
#define BOOT_TARGET_DEVICES_USB(func) func(USB, usb, 0)
#else
#define BOOT_TARGET_DEVICES_USB(func)
#endif


#include <config_distro_bootcmd.h>

#ifdef CONFIG_ARM64
#define FDTFILE "allwinner/" CONFIG_DEFAULT_DEVICE_TREE ".dtb"
#else
#define FDTFILE CONFIG_DEFAULT_DEVICE_TREE ".dtb"
#endif

#define CONFIG_BOOTCOUNT_ENV \
	"bootcount=0\0" \
	"boottest=0\0" \
	"boottestcount=if test $boottest = 1; then " \
		"setexpr bootcount ${bootcount} + 1; " \
		"saveenv; " \
		"echo boottest=1 bootcount: ${bootcount}; " \
		"fi;\0"

#define CONFIG_EXTRA_ENV_SETTINGS \
	CONFIG_BOOTCOUNT_ENV \
	"earlyprintk=sunxi-uart,0x05000000\0" \
	"initcall_debug=0\0" \
	"console=ttyS0,115200\0" \
	"nand_root=/dev/nand0p4\0" \
	"nand_rootbak=/dev/nand0p5\0" \
	"ubinand_root=/dev/ubi0_4\0" \
	"emmc_root=/dev/mmcblk0p4 rootwait rw\0" \
	"mmc_rootbak=/dev/mmcblk0p5\0" \
	"sd_root=/dev/mmcblk1p4 rootwait rw\0" \
	"nor_root=/dev/mtdblock2\0" \
	"root_partition=rootfs\0" \
	"mtd_name=sys\0" \
	"init=/init\0" \
	"loglevel=8\0" \
	"selinux=0\0" \
	"cma=64M\0" \
	"mac=\0" \
	"mac1=\0" \
	"wifi_mac=\0" \
	"bt_mac=\0" \
	"specialstr=\0" \
	"keybox_list=hdcpkey,widevine\0" \
	"setargs_nor=setenv bootargs  earlyprintk=${earlyprintk} initcall_debug=${initcall_debug} console=${console} loglevel=${loglevel} root=${nor_root} init=${init} partitions=${partitions} cma=${cma} selinux=${selinux}\0" \
	"setargs_nand=setenv bootargsubi.mtd=${mtd_name} earlyprintk=${earlyprintk} initcall_debug=${initcall_debug} console=${console} loglevel=${loglevel} root=${nand_root} init=${init} partitions=${partitions} cma=${cma} snum=${snum} mac_addr=${mac}   mac1_addr=${mac1} wifi_mac=${wifi_mac} bt_mac=${bt_mac} selinux=${selinux} specialstr=${specialstr} gpt=1 rootfstype=${rootfstype}\0" \
	"setargs_nand_ubi=setenv bootargs earlyprintk=${earlyprintk} initcall_debug=${initcall_debug} console=${console} loglevel=${loglevel} ubi.mtd=${mtd_name} ubi.block=0,${root_partition} root=${ubinand_root} init=${init} partitions=${partitions} cma=${cma} mac_addr=${mac} selinux=${selinux} specialstr=${specialstr} snum=${snum}\0" \
	"setargs_emmc=setenv  bootargs earlyprintk=${earlyprintk} initcall_debug=${initcall_debug} console=${console} loglevel=${loglevel} root=${emmc_root}  init=${init} partitions=${partitions} cma=${cma} snum=${snum} mac_addr=${mac}   mac1_addr=${mac1} wifi_mac=${wifi_mac} bt_mac=${bt_mac} selinux=${selinux} specialstr=${specialstr} gpt=1\0" \
	"setargs_sd=setenv  bootargs earlyprintk=${earlyprintk} initcall_debug=${initcall_debug} console=${console} loglevel=${loglevel} root=${sd_root}  init=${init} partitions=${partitions} cma=${cma} snum=${snum} mac_addr=${mac}   mac1_addr=${mac1} wifi_mac=${wifi_mac} bt_mac=${bt_mac} selinux=${selinux} specialstr=${specialstr} gpt=1\0" \
	"setargs_netboot=setenv bootargs earlyprintk=${earlyprintk} initcall_debug=${initcall_debug} console=${console} loglevel=${loglevel} root=/dev/nfs rw rootwait ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}::eth0:off nfsroot=${serverip}:${nfsroot},nolock,v3,tcp init=${init} partitions=${partitions} cma=${cma} snum=${snum} mac_addr=${mac} mac1_addr=${mac1} wifi_mac=${wifi_mac} bt_mac=${bt_mac} selinux=${selinux} specialstr=${specialstr} gpt=1'\0" \
	"boot_normal=sunxi_flash read 45000000 boot;bootm 45000000\0" \
	"boot_recovery=sunxi_flash read 45000000 recovery;bootm 45000000\0" \
	"boot_net=tftpboot 45000000 boot.fex; run setargs_netboot; bootm 45000000\0" \
	"boot_fastboot=fastboot\0" \
	"recovery_key_value_max=0x13\0" \
	"recovery_key_value_min=0x10\0" \
	"fastboot_key_value_max=0x8\0" \
	"fastboot_key_value_min=0x2\0"
#else /* ifndef CONFIG_SPL_BUILD */
#define CONFIG_EXTRA_ENV_SETTINGS

#endif

#define SUNXI_SPRITE_ENV_SETTINGS	\
	"bootdelay=0\0" \
	"bootcmd=run sunxi_sprite_test\0" \
	"console=ttyS0,115200\0" \
	"sunxi_sprite_test=sprite_test read\0"

#define CONFIG_BOARD_LATE_INIT
#define CONFIG_BOARD_EARLY_INIT_R

#endif /* _SUNXI_COMMON_CONFIG_H */
