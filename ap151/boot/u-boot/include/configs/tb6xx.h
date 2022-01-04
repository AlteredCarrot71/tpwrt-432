/*
 * Copyright (c) 2013 Qualcomm Atheros, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __TB6XX_H
#define __TB6XX_H

#include <config.h>

#undef MTDPARTS_DEFAULT

#undef CFG_HZ

#include <atheros.h>

#ifndef FLASH_SIZE
#define FLASH_SIZE 8
#endif
/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks */
#if (FLASH_SIZE == 16)
#define CFG_MAX_FLASH_SECT	256	/* max number of sectors on one chip */
#elif (FLASH_SIZE == 8)
#define CFG_MAX_FLASH_SECT	128	/* max number of sectors on one chip */
#else
#define CFG_MAX_FLASH_SECT	64	/* max number of sectors on one chip */
#endif

#define CFG_FLASH_SECTOR_SIZE	(64*1024)
#if (FLASH_SIZE == 16)
#define CFG_FLASH_SIZE		0x01000000	/* Total flash size */
#elif (FLASH_SIZE == 8)
#define CFG_FLASH_SIZE		0x00800000	/* max number of sectors on one chip */
#else
#define CFG_FLASH_SIZE		0x00400000	/* Total flash size */
#endif

#ifndef COMPRESSED_UBOOT
#define ENABLE_DYNAMIC_CONF	1
#endif

//#define CONFIG_WASP_SUPPORT	1
#undef CFG_ATHRS26_PHY

#if (CFG_MAX_FLASH_SECT * CFG_FLASH_SECTOR_SIZE) != CFG_FLASH_SIZE
#	error "Invalid flash configuration"
#endif

#define CFG_FLASH_WORD_SIZE	unsigned short

// Set this to zero, for 16bit ddr2
#define DDR2_32BIT_SUPPORT 		1

/* S16 specific defines */

#define CONFIG_AG7240_GE0_IS_CONNECTED	1

/* NOR Flash start address */
#define CFG_FLASH_BASE			0x9f000000

#ifdef COMPRESSED_UBOOT
#define BOOTSTRAP_TEXT_BASE		CFG_FLASH_BASE
#define BOOTSTRAP_CFG_MONITOR_BASE	BOOTSTRAP_TEXT_BASE
#endif

/*
 * Defines to change flash size on reboot
 */
#ifdef ENABLE_DYNAMIC_CONF
#define UBOOT_FLASH_SIZE		(256 * 1024)
#define UBOOT_ENV_SEC_START		(CFG_FLASH_BASE + UBOOT_FLASH_SIZE)

#define CFG_FLASH_MAGIC			0xaabacada
#define CFG_FLASH_MAGIC_F		(UBOOT_ENV_SEC_START + CFG_FLASH_SECTOR_SIZE - 0x20)
#define CFG_FLASH_SECTOR_SIZE_F		*(volatile int *)(CFG_FLASH_MAGIC_F + 0x4)
#define CFG_FLASH_SIZE_F		*(volatile int *)(CFG_FLASH_MAGIC_F + 0x8) /* Total flash size */
#define CFG_MAX_FLASH_SECT_F		(CFG_FLASH_SIZE / CFG_FLASH_SECTOR_SIZE) /* max number of sectors on one chip */
#else
#define CFG_FLASH_SIZE_F		CFG_FLASH_SIZE
#define CFG_FLASH_SECTOR_SIZE_F		CFG_FLASH_SECTOR_SIZE
#endif


#define CFG_DDR_REFRESH_VAL			0x412c

/*
 * The following #defines are needed to get flash environment right
 */
#define	CFG_MONITOR_BASE	TEXT_BASE
#define	CFG_MONITOR_LEN		(192 << 10)

#undef CONFIG_BOOTARGS

#if CONFIG_MI124
#define BC "bc=mi124_f1e\0"
#endif
#if CONFIG_AP123
#define BC "bc=ap123\0"
#endif

#define __gen_cmd(n, a, f, ec, cc, el)		\
	#n "=tftp 0x80060000 ${dir}" #f "&&"	\
	#ec " " #a " " #el "&&"			\
	#cc " $fileaddr " #a " $filesize\0"

#define gen_cmd(n, a, f)			\
	__gen_cmd(n, a, f, erase, cp.b, +$filesize)

#define gen_cmd_el(n, a, f, el)			\
	__gen_cmd(n, a, f, erase, cp.b, +el)

#define nand_gen_cmd(n, a, f, s)		\
	__gen_cmd(n, a, f, nand erase, nand write, s)

#ifdef CONFIG_ATH_NAND_SUPPORT
#	ifdef CONFIG_ATH_NAND_BR	// nand boot rom
#		define ATH_U_CMD	nand_gen_cmd(lu, 0x0, 2fw.bin, 0x40000)
#		define MTDPARTS_DEFAULT "mtdparts=ath-nand:256k(u-boot),256k(u-boot-env),1280k(uImage),7m(rootfs),128k(dummy),128k(caldata)"
#		define ATH_ROOT_DEV	"31:03"
#		define CFG_ENV_ADDR	0x00040000
#	else	//dual flash
#		define ATH_U_CMD	gen_cmd(lu, 0x9f000000, u-boot.bin)
#	define MTDPARTS_DEFAULT 	"mtdparts=ath-nor0:320k(u-boot-and-env),6336k(free);ath-nand:256k(u-boot),256k(u-boot-env),1280k(uImage),7m(rootfs),128k(dummy),128k(caldata)"
#		define ATH_ROOT_DEV	"31:05"
#		define CFG_ENV_ADDR	0x9f040000
#	endif
#	define ATH_F_FILE		tb6xx${bc}-nand-jffs2
#	define ATH_F_LEN		0x700000
#	define ATH_F_ADDR		0x1c0000
#	define ATH_K_FILE		vmlinux${bc}.lzma.uImage
#	define ATH_K_ADDR		0x80000
#	define ATH_F_CMD		nand_gen_cmd(lf, ATH_F_ADDR, ATH_F_FILE, ATH_F_LEN)
#	define ATH_K_CMD		nand_gen_cmd(lk, ATH_K_ADDR, ATH_K_FILE, 0x140000)
#	define ATH_EXTRA_ENV		"bootdevice=0\0"
#	define CONFIG_BOOTCOMMAND	"nboot 0x81000000 0 0x80000"
#elif defined(CONFIG_MI124) || defined(CONFIG_AP123)
#		ifdef COMPRESSED_UBOOT
#		define ATH_U_FILE	tuboot.bin
#		define ATH_F_FILE	${bc}-jffs2
#		define ATH_F_LEN	$filesize
#		define ATH_F_ADDR	0x9f010000
#		define ATH_K_FILE	vmlinux_${bc}.lzma.uImage
#		define ATH_K_ADDR	0x9f300000
			/*
			 * For compressed uboot, environment sector is not used.
			 * Hence the mtd partition indices get reduced by 1.
			 * This conflicts with
			 *	- minor no. for /dev/caldata in
			 *		build/scripts/{board}/dev.txt
			 *	- root=<rooot dev> kernel cmdline parameter
			 * Hence, doing a dummy split of the u-boot partition
			 * to maintain the same minor no. as in the normal u-boot.
			 */
#			define MTDPARTS_DEFAULT		"mtdparts=ath-nor0:32k(u-boot1),32k(u-boot2),3008k(rootfs),896k(uImage),64k(mib0),64k(ART)"
#		else /* COMPRESSED_UBOOT */
#		define ATH_U_FILE	u-boot.bin
#		define ATH_F_FILE	${bc}-jffs2
#		define ATH_F_LEN	$filesize
#		define ATH_F_ADDR	0x9f050000
#		define ATH_K_FILE	vmlinux_${bc}.lzma.uImage
#		define ATH_K_ADDR	0x9f300000
#			define MTDPARTS_DEFAULT		"mtdparts=ath-nor0:256k(u-boot),64k(u-boot-env),2752k(rootfs),896k(uImage),64k(mib0),64k(ART)"
#		endif /* COMPRESSED_UBOOT */
#elif defined(CONFIG_TB6XX_P2P)
#	define ATH_U_FILE		u-boot.bin
#	define ATH_F_FILE		tb6xx-p2p${bc}-jffs2
#	define ATH_F_LEN		0x630000
#	define ATH_F_ADDR		0x9f050000
#	define ATH_K_FILE		vmlinux${bc}.lzma.uImage
#	define ATH_K_ADDR		0x9f680000
#	define MTDPARTS_DEFAULT		"mtdparts=ath-nor0:256k(u-boot),64k(u-boot-env),6336k(rootfs),1408k(uImage),64k(mib0),64k(ART)"
#else
#	define ATH_U_FILE		u-boot.bin
#	define ATH_F_FILE		tb6xx${bc}-jffs2
#	define ATH_F_LEN		0x630000
#	define ATH_F_ADDR		0x9f050000
#	define ATH_K_FILE		vmlinux${bc}.lzma.uImage
#	define ATH_K_ADDR		0x9f680000
#	define MTDPARTS_DEFAULT		"mtdparts=ath-nor0:256k(u-boot),64k(u-boot-env),6336k(rootfs),1408k(uImage),64k(mib0),64k(ART)"
#endif /*CONFIG_MI124*/

#ifndef ATH_ROOT_DEV
#	define ATH_ROOT_DEV	"31:02"
#endif

#ifndef ATH_EXTRA_ENV
#	define ATH_EXTRA_ENV
#endif

#ifndef ATH_U_CMD
#	define ATH_U_CMD	gen_cmd(lu, 0x9f000000, ATH_U_FILE)
#endif

#ifndef ATH_F_CMD
#	define ATH_F_CMD	gen_cmd_el(lf, ATH_F_ADDR, ATH_F_FILE, ATH_F_LEN)
#endif

#ifndef ATH_K_CMD
#	define ATH_K_CMD	gen_cmd(lk, ATH_K_ADDR, ATH_K_FILE)
#endif

#define CONFIG_EXTRA_ENV_SETTINGS	\
	"dir=\0" ATH_U_CMD ATH_F_CMD ATH_K_CMD ""

#define	CONFIG_BOOTARGS		"console=ttyS0,115200 root=" ATH_ROOT_DEV " rootfstype=jffs2 init=/sbin/init " MTDPARTS_DEFAULT

//#define CFG_PLL_FREQ    CFG_PLL_720_600_200

/*
 * timeout values are in ticks
 */
#define CFG_FLASH_ERASE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Write */

/*
 * Cache lock for stack
 */
#define CFG_INIT_SP_OFFSET	0x1000

#ifndef CONFIG_ATH_NAND_SUPPORT
#	define CFG_ENV_ADDR		0x9f040000
#	if (FLASH_SIZE == 16)
#		define CONFIG_BOOTCOMMAND "bootm 0x9f550000"
#	else	/* FLASH_SIZE == 16 */
#		ifdef COMPRESSED_UBOOT
#			if (FLASH_SIZE == 4)
#				define CONFIG_BOOTCOMMAND "bootm 0x9f300000"
#			else	/* FLASH_SIZE == 4 */
#				define CONFIG_BOOTCOMMAND "bootm 0x9f320000"
#			endif	/* FLASH_SIZE == 4 */
#		else /* COMPRESSED_UBOOT */
#			if defined(CONFIG_MI124) || defined(CONFIG_AP123)
#				define CONFIG_BOOTCOMMAND "bootm 0x9f300000"
#			else
#				define CONFIG_BOOTCOMMAND "bootm 0x9f680000"
#			endif
#		endif	/* COMPRESSED_UBOOT */
#	endif	/* FLASH_SIZE == 16 */
#endif



#ifdef ENABLE_DYNAMIC_CONF
#define CFG_DDR_MAGIC		0xaabacada
#define CFG_DDR_MAGIC_F		(UBOOT_ENV_SEC_START + CFG_FLASH_SECTOR_SIZE - 0x30)
#define CFG_DDR_CONFIG_VAL_F	*(volatile int *)(CFG_DDR_MAGIC_F + 4)
#define CFG_DDR_CONFIG2_VAL_F	*(volatile int *)(CFG_DDR_MAGIC_F + 8)
#define CFG_DDR_EXT_MODE_VAL_F	*(volatile int *)(CFG_DDR_MAGIC_F + 12)
#endif

#define CONFIG_NET_MULTI
#define CONFIG_MEMSIZE_IN_BYTES
#define CONFIG_PCI 1

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#ifndef COMPRESSED_UBOOT
#define ATH_CFG_COMMANDS	((			\
				CONFIG_CMD_DFL	|	\
				CFG_CMD_DHCP	|	\
				CFG_CMD_ELF	|	\
				CFG_CMD_PCI	|	\
				CFG_CMD_FLS	|	\
				CFG_CMD_MII	|	\
				CFG_CMD_PING	|	\
				CFG_CMD_NET	|	\
				CFG_CMD_ENV	|	\
				CFG_CMD_PLL	|	\
				CFG_CMD_FLASH	|	\
				CFG_CMD_RUN	|	\
				CFG_CMD_ELF	|	\
				CFG_CMD_DDR	|	\
				CFG_CMD_ETHREG		\
				) & ~(			\
				CFG_CMD_IMLS	|	\
				CFG_CMD_FLASH		\
				))
#else
#define ATH_CFG_COMMANDS	(CONFIG_CMD_DFL	|	\
				CFG_CMD_PING	|	\
				CFG_CMD_NET)
#endif /* #ifndef COMPRESSED_UBOOT */

#ifdef CONFIG_ATH_NAND_SUPPORT
#	ifdef CONFIG_ATH_NAND_BR
#		define CFG_ENV_IS_IN_NAND	1
#		define CFG_ENV_OFFSET		0x40000u
#		define CFG_ENV_SIZE		0x40000u
#		define ATH_EXTRA_CMD		CFG_CMD_NAND
#	else
#		define CFG_ENV_IS_IN_FLASH	1
#		define CFG_ENV_SIZE		CFG_FLASH_SECTOR_SIZE
#		define ATH_EXTRA_CMD		(CFG_CMD_NAND | CFG_CMD_FLASH)
#	endif
#	define NAND_MAX_CHIPS			1
#	define CFG_MAX_NAND_DEVICE		1
#else
#	define ATH_EXTRA_CMD			CFG_CMD_FLASH
#	define CFG_ENV_IS_IN_FLASH		1
#	define CFG_ENV_SIZE			CFG_FLASH_SECTOR_SIZE
#endif

#define DEBUG

#ifdef COMPRESSED_UBOOT
#undef  CFG_ENV_IS_IN_FLASH
#undef  CFG_ENV_IS_IN_NAND
#define CFG_ENV_IS_NOWHERE		1
#endif

#define CONFIG_COMMANDS			(ATH_CFG_COMMANDS | ATH_EXTRA_CMD)

#define CONFIG_IPADDR			192.168.1.1
#define CONFIG_SERVERIP			192.168.1.10
#define CONFIG_ETHADDR			0x00:0xaa:0xbb:0xcc:0xdd:0xee
#define CFG_FAULT_ECHO_LINK_DOWN	1

#define CFG_PHY_ADDR			0
#define CFG_GMII			0
#define CFG_MII0_RMII			1
#define CFG_AG7100_GE0_RMII		1

#define CFG_BOOTM_LEN			(16 << 20) /* 16 MB */
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2		"hush>"

/*
** Parameters defining the location of the calibration/initialization
** information for the two Merlin devices.
** NOTE: **This will change with different flash configurations**
*/

#define WLANCAL				0x9fff1000
#define BOARDCAL			0x9fff0000
#define ATHEROS_PRODUCT_ID		137
#define CAL_SECTOR			(CFG_MAX_FLASH_SECT - 1)

/* For Merlin, both PCI, PCI-E interfaces are valid */
#define ATH_ART_PCICFG_OFFSET		12

#include <cmd_confdefs.h>

#endif	/* __TB6XX_H */
