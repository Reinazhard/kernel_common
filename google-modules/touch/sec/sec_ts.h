/* drivers/input/touchscreen/sec_ts.h
 *
 * Copyright (C) 2015 Samsung Electronics Co., Ltd.
 * http://www.samsungsemi.com/
 *
 * Core file for Samsung TSC driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __SEC_TS_H__
#define __SEC_TS_H__

#include <asm/unaligned.h>
#include <linux/completion.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/gpio.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/spi/spi.h>
#include <linux/input.h>
#if IS_ENABLED(CONFIG_TOUCHSCREEN_HEATMAP)
#include <heatmap.h>
#endif
#include <linux/input/mt.h>
#if IS_ENABLED(CONFIG_TOUCHSCREEN_OFFLOAD)
#include <touch_offload.h>
#endif
#include "sec_cmd.h"
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/module.h>
#include <drm/drm_bridge.h>
#include <drm/drm_device.h>
#include <drm/drm_encoder.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/pm_qos.h>
#include <linux/power_supply.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/workqueue.h>
#ifdef CONFIG_SEC_SYSFS
#include <linux/sec_sysfs.h>
#endif

#ifdef CONFIG_INPUT_BOOSTER
#include <linux/input/input_booster.h>
#endif

#if IS_ENABLED(CONFIG_TOUCHSCREEN_TBN)
#include <touch_bus_negotiator.h>
#endif

#define SEC_TS_NAME		"sec_ts"
#define SEC_TS_DEVICE_NAME	"SEC_TS"

#undef SEC_TS_DEBUG_IO
#undef USE_OPEN_CLOSE
#undef USE_RESET_DURING_POWER_ON
#undef USE_RESET_EXIT_LPM
#undef USE_POR_AFTER_I2C_RETRY
#undef USE_POR_AFTER_SPI_RETRY
#undef USER_OPEN_DWORK
#undef USE_PRESSURE_SENSOR
#undef PAT_CONTROL
#define USE_CHARGER_WORK
#undef USE_STIM_PAD
#undef USE_SPEC_CHECK

#if defined(USE_RESET_DURING_POWER_ON) || defined(USE_POR_AFTER_I2C_RETRY) || \
    defined(USE_RESET_EXIT_LPM) || defined(USE_POR_AFTER_SPI_RETRY)
#define USE_POWER_RESET_WORK
#endif

#ifndef I2C_INTERFACE
#define SPI_CLOCK_FREQ			10000000
#define SPI_DELAY_CS			10
#define SEC_TS_SPI_SYNC_CODE		0xAA
#define SEC_TS_SPI_HEADER_SIZE		5
#define SEC_TS_SPI_READ_HEADER_SIZE	7
#define SEC_TS_SPI_CHECKSUM_SIZE	1

#define SEC_TS_SPI_CMD_OK		0x0
#define SEC_TS_SPI_CMD_NG		(1u<<7)
#define SEC_TS_SPI_CMD_UNKNOWN		(SEC_TS_SPI_CMD_NG | (1))
#define SEC_TS_SPI_CMD_FAIL		(SEC_TS_SPI_CMD_NG | (2))
#define SEC_TS_SPI_CMD_BAD_PARAM	(SEC_TS_SPI_CMD_NG | (3))
#define SEC_TS_SPI_CMD_CHKSUM_FAIL	(SEC_TS_SPI_CMD_NG | (4))
#endif

#define TOUCH_RESET_DWORK_TIME		10
#define BRUSH_Z_DATA			63	/* for ArtCanvas */

#define MASK_1_BITS			0x0001
#define MASK_2_BITS			0x0003
#define MASK_3_BITS			0x0007
#define MASK_4_BITS			0x000F
#define MASK_5_BITS			0x001F
#define MASK_6_BITS			0x003F
#define MASK_7_BITS			0x007F
#define MASK_8_BITS			0x00FF

/* support feature */
//#define SEC_TS_SUPPORT_CUSTOMLIB	/* support user defined library */

#define TYPE_STATUS_EVENT_CMD_DRIVEN	0
#define TYPE_STATUS_EVENT_ERR		1
#define TYPE_STATUS_EVENT_INFO		2
#define TYPE_STATUS_EVENT_USER_INPUT	3
#define TYPE_STATUS_EVENT_CUSTOMLIB_INFO	6
#define TYPE_STATUS_EVENT_VENDOR_INFO	7
#define TYPE_STATUS_CODE_SAR	0x28

#define BIT_STATUS_EVENT_CMD_DRIVEN(a)	(a << TYPE_STATUS_EVENT_CMD_DRIVEN)
#define BIT_STATUS_EVENT_ERR(a)		(a << TYPE_STATUS_EVENT_ERR)
#define BIT_STATUS_EVENT_INFO(a)	(a << TYPE_STATUS_EVENT_INFO)
#define BIT_STATUS_EVENT_USER_INPUT(a)	(a << TYPE_STATUS_EVENT_USER_INPUT)
#define BIT_STATUS_EVENT_VENDOR_INFO(a)	(a << TYPE_STATUS_EVENT_VENDOR_INFO)

#define DO_FW_CHECKSUM			(1 << 0)
#define DO_PARA_CHECKSUM		(1 << 1)
#define MAX_SUPPORT_TOUCH_COUNT		10
#define MAX_SUPPORT_HOVER_COUNT		1

#define SEC_TS_EVENTID_HOVER		10

#define SEC_TS_DEFAULT_FW_NAME		"tsp_sec/sec_hero.fw"
#define SEC_TS_DEFAULT_BL_NAME		"tsp_sec/s6smc41_blupdate_img_REL.bin"
#define SEC_TS_DEFAULT_PARA_NAME	"tsp_sec/s6smc41_para_REL_DGA0_V0106_150114_193317.bin"
#define SEC_TS_DEFAULT_FFU_FW		"ffu_tsp.bin"
#define SEC_TS_MAX_FW_PATH		64
#define SEC_TS_FW_BLK_SIZE_MAX		(512)
#define SEC_TS_FW_BLK_SIZE_DEFAULT	(512)
#define SEC_TS_SELFTEST_REPORT_SIZE	80
#define SEC_TS_PRESSURE_MAX		0x3f

#define IO_WRITE_BUFFER_SIZE		(256 - 1)//10

#ifdef I2C_INTERFACE
/* max read size: from sec_ts_read_event() at sec_ts.c */
#define IO_PREALLOC_READ_BUF_SZ	(32 * SEC_TS_EVENT_BUFF_SIZE)
/* max write size: from sec_ts_flashpagewrite() at sec_ts_fw.c */
#define IO_PREALLOC_WRITE_BUF_SZ	(SEC_TS_SPI_HEADER_SIZE + 1 + 2 +\
					    SEC_TS_FW_BLK_SIZE_MAX + 1)
#else
#define IO_PREALLOC_READ_BUF_SZ	2048
#define IO_PREALLOC_WRITE_BUF_SZ	1024
#endif

#define SEC_TS_FW_HEADER_SIGN		0x53494654
#define SEC_TS_FW_CHUNK_SIGN		0x53434654

#undef SEC_TS_FW_UPDATE_ON_PROBE
#define SEC_TS_FW_UPDATE_DELAY_MS_AFTER_PROBE	1000

#define AMBIENT_CAL			0
#define OFFSET_CAL_SDC			1
#define OFFSET_CAL_SEC			2
#define PRESSURE_CAL			3

#define SEC_TS_SKIPTSP_DUTY		100

#define SEC_TS_NVM_OFFSET_FAC_RESULT		0
#define SEC_TS_NVM_OFFSET_CAL_COUNT		1
#define SEC_TS_NVM_OFFSET_DISASSEMBLE_COUNT	2
#define SEC_TS_NVM_OFFSET_TUNE_VERSION		3
#define SEC_TS_NVM_OFFSET_TUNE_VERSION_LENGTH	2

#define SEC_TS_NVM_OFFSET_PRESSURE_INDEX	5

#define SEC_TS_NVM_OFFSET_PRESSURE_STRENGTH	6
#define SEC_TS_NVM_OFFSET_PRESSURE_STRENGTH_1	6
#define SEC_TS_NVM_OFFSET_PRESSURE_STRENGTH_2	12
#define SEC_TS_NVM_OFFSET_PRESSURE_STRENGTH_3	18
#define SEC_TS_NVM_OFFSET_PRESSURE_STRENGTH_4	24

#define SEC_TS_NVM_OFFSET_PRESSURE_RAWDATA	30
#define SEC_TS_NVM_OFFSET_PRESSURE_RAWDATA_1	30
#define SEC_TS_NVM_OFFSET_PRESSURE_RAWDATA_2	36
#define SEC_TS_NVM_OFFSET_PRESSURE_RAWDATA_3	42
#define SEC_TS_NVM_OFFSET_PRESSURE_RAWDATA_4	48
#define SEC_TS_NVM_SIZE_PRESSURE_BLOCK		6

#define SEC_TS_NVM_OFFSET_PRESSURE_BASE_CAL_COUNT	54
#define SEC_TS_NVM_OFFSET_PRESSURE_DELTA_CAL_COUNT	55
#define SEC_TS_NVM_SIZE_PRESSURE_CAL_BLOCK		1

#define SEC_TS_NVM_LAST_BLOCK_OFFSET	\
	    SEC_TS_NVM_OFFSET_PRESSURE_DELTA_CAL_COUNT
#define SEC_TS_NVM_LAST_BLOCK_SIZE	SEC_TS_NVM_SIZE_PRESSURE_CAL_BLOCK

#define SEC_TS_NVM_OFFSET_LENGTH	(SEC_TS_NVM_LAST_BLOCK_OFFSET +\
					    SEC_TS_NVM_LAST_BLOCK_SIZE + 1)

/* SEC_TS READ REGISTER ADDRESS */
#define SEC_TS_CMD_SENSE_ON			0x10
#define SEC_TS_CMD_SENSE_OFF			0x11
#define SEC_TS_CMD_SW_RESET			0x12
#define SEC_TS_CMD_CALIBRATION_SEC		0x13	/* send it to touch ic,
							 * but touch ic works
							 * nothing.
							 **/
#define SEC_TS_CMD_FACTORY_PANELCALIBRATION	0x14

#define SEC_TS_READ_GPIO_STATUS			0x20	// not support
#define SEC_TS_READ_FIRMWARE_INTEGRITY		0x21
#define SEC_TS_READ_DEVICE_ID			0x22
#define SEC_TS_READ_PANEL_INFO			0x23
#define SEC_TS_READ_CORE_CONFIG_VERSION		0x24
#define SEC_TS_CMD_DISABLE_GAIN_LIMIT		0x2A

#define SEC_TS_CMD_SET_TOUCHFUNCTION		0x30
#define SEC_TS_CMD_SET_TSC_MODE			0x31
#define SET_TS_CMD_SET_CHARGER_MODE		0x32
#define SET_TS_CMD_SET_NOISE_MODE		0x33
#define SET_TS_CMD_SET_REPORT_RATE		0x34
#define SEC_TS_CMD_TOUCH_MODE_FOR_THRESHOLD	0x35
#define SEC_TS_CMD_TOUCH_THRESHOLD		0x36
#define SET_TS_CMD_KEY_THRESHOLD		0x37
#define SEC_TS_CMD_SET_COVERTYPE		0x38
#define SEC_TS_CMD_WAKEUP_GESTURE_MODE		0x39
#define SEC_TS_WRITE_POSITION_FILTER		0x3A
#define SEC_TS_CMD_WET_MODE			0x3B
#define SEC_TS_CMD_DISABLE_NORM_TABLE		0x40
#define SEC_TS_CMD_READ_NORM_TABLE		0x41
#define SEC_TS_CMD_DISABLE_BASELINE_ADAPT	0x43
#define SEC_TS_CMD_DISABLE_DF			0x44
#define SEC_TS_CMD_ERASE_FLASH			0x45
#define SEC_TS_CMD_RESET_BASELINE		0x47
#define SEC_TS_CMD_SET_CONT_REPORT		0x49
#define SEC_TS_CMD_WRITE_NORM_TABLE		0x49
#if IS_ENABLED(CONFIG_TOUCHSCREEN_HEATMAP)
#define SEC_TS_CMD_HEATMAP_READ			0x4A
#define SEC_TS_CMD_HEATMAP_ENABLE		0x4B
#endif
#define SEC_TS_READ_ID				0x52
#define SEC_TS_READ_BOOT_STATUS			0x55
#define SEC_TS_CMD_ENTER_FW_MODE		0x57
#define SEC_TS_READ_ONE_EVENT			0x60
#define SEC_TS_READ_ALL_EVENT			0x61
#define SEC_TS_CMD_CLEAR_EVENT_STACK		0x62
#define SEC_TS_CMD_MUTU_RAW_TYPE		0x70
#define SEC_TS_CMD_SELF_RAW_TYPE		0x71
#define SEC_TS_READ_TOUCH_RAWDATA		0x72
#define SEC_TS_READ_TOUCH_SELF_RAWDATA		0x73
#define SEC_TS_READ_SELFTEST_RESULT		0x80
#define SEC_TS_CMD_CALIBRATION_AMBIENT		0x81
#define SEC_TS_CMD_P2PTEST			0x82
#define SEC_TS_CMD_SET_P2PTEST_MODE		0x83
#define SEC_TS_CMD_NVM				0x85
#define SEC_TS_CMD_SET_WET_MODE			0x8B
#define SEC_TS_CMD_STATEMANAGE_ON		0x8E
#define SEC_TS_CMD_CALIBRATION_OFFSET_SDC	0x8F

/* SEC_TS CUSTOMLIB OPCODE COMMAND */
#define SEC_TS_CMD_CUSTOMLIB_GET_INFO			0x90
#define SEC_TS_CMD_CUSTOMLIB_WRITE_PARAM			0x91
#define SEC_TS_CMD_CUSTOMLIB_READ_PARAM			0x92
#define SEC_TS_CMD_CUSTOMLIB_NOTIFY_PACKET			0x93
#define SEC_TS_CMD_CUSTOMLIB_OFFSET_PRESSURE_LEVEL		0x5E
#define SEC_TS_CMD_CUSTOMLIB_OFFSET_PRESSURE_THD_HIGH	0x84
#define SEC_TS_CMD_CUSTOMLIB_OFFSET_PRESSURE_THD_LOW	0x86
#define SEC_TS_CMD_CUSTOMLIB_LP_DUMP			0x01F0

#define SEC_TS_CMD_STATUS_EVENT_TYPE	0xA0
#define SEC_TS_READ_FW_INFO		0xA2
#define SEC_TS_READ_FW_VERSION		0xA3
#define SEC_TS_READ_PARA_VERSION	0xA4
#define SEC_TS_READ_IMG_VERSION		0xA5
#define SEC_TS_CMD_GET_CHECKSUM		0xA6
#define SEC_TS_CMD_MIS_CAL_CHECK	0xA7
#define SEC_TS_CMD_MIS_CAL_READ		0xA8
#define SEC_TS_CMD_MIS_CAL_SPEC		0xA9
#define SEC_TS_CMD_DEADZONE_RANGE	0xAA
#define SEC_TS_CMD_LONGPRESSZONE_RANGE	0xAB
#define SEC_TS_CMD_LONGPRESS_DROP_AREA	0xAC
#define SEC_TS_CMD_LONGPRESS_DROP_DIFF	0xAD
#define SEC_TS_READ_TS_STATUS		0xAF
#define SEC_TS_CMD_SELFTEST		0xAE
#define SEC_TS_READ_FORCE_RECAL_COUNT	0xB0
#define SEC_TS_READ_FORCE_SIG_MAX_VAL	0xB1
#define SEC_TS_CAAT_READ_STORED_DATA	0xB7
#define SEC_TS_CMD_SET_NOISE_MODE	0xBB
#define SEC_TS_CMD_SET_GRIP_DETEC	0xBC
#define SEC_TS_CMD_SET_PALM_DETEC	0xBE
#define SEC_TS_READ_CSRAM_RTDP_DATA	0xC3

/* SEC_TS FLASH COMMAND */
#define SEC_TS_CMD_FLASH_READ_ADDR	0xD0
#define SEC_TS_CMD_FLASH_READ_SIZE	0xD1
#define SEC_TS_CMD_FLASH_READ_DATA	0xD2
#define SEC_TS_CMD_CHG_SYSMODE		0xD7
#define SEC_TS_CMD_FLASH_ERASE		0xD8
#define SEC_TS_CMD_FLASH_WRITE		0xD9
#define SEC_TS_CMD_FLASH_PADDING	0xDA

#define SEC_TS_READ_BL_UPDATE_STATUS	0xDB
#define SEC_TS_CMD_SET_TOUCH_ENGINE_MODE	0xE1
#define SEC_TS_CMD_SET_POWER_MODE	0xE4
#define SEC_TS_CMD_EDGE_DEADZONE	0xE5
#define SEC_TS_CMD_SET_DEX_MODE		0xE7
#define SEC_TS_CMD_CALIBRATION_PRESSURE		0xE9
/* Have to need delay 30msec after writing 0xEA command */
/* Do not write Zero with 0xEA command */
#define SEC_TS_CMD_SET_GET_PRESSURE		0xEA
#define SEC_TS_CMD_SET_USER_PRESSURE		0xEB
#define SEC_TS_CMD_SET_TEMPERATURE_COMP_MODE	0xEC
#define SEC_TS_CMD_SET_TOUCHABLE_AREA		0xED
#define SEC_TS_CMD_SET_BRUSH_MODE		0xEF

#define SEC_TS_READ_CALIBRATION_REPORT		0xF1
#define SEC_TS_CMD_SET_VENDOR_EVENT_LEVEL	0xF2
#define SEC_TS_CMD_SET_SPENMODE			0xF3
#define SEC_TS_CMD_SELECT_PRESSURE_TYPE		0xF5
#define SEC_TS_CMD_READ_PRESSURE_DATA		0xF6

#define SEC_TS_FLASH_SIZE_64		64
#define SEC_TS_FLASH_SIZE_128		128
#define SEC_TS_FLASH_SIZE_256		256

#define SEC_TS_FLASH_SIZE_CMD		1
#define SEC_TS_FLASH_SIZE_ADDR		2
#define SEC_TS_FLASH_SIZE_CHECKSUM	1

#define SEC_TS_STATUS_BOOT_MODE		0x10
#define SEC_TS_STATUS_APP_MODE		0x20

#define SEC_TS_FIRMWARE_PAGE_SIZE_256	256
#define SEC_TS_FIRMWARE_PAGE_SIZE_128	128

/* SEC status event id */
#define SEC_TS_COORDINATE_EVENT		0
#define SEC_TS_STATUS_EVENT		1
#define SEC_TS_GESTURE_EVENT		2
#define SEC_TS_EMPTY_EVENT		3

#define SEC_TS_EVENT_BUFF_SIZE		8
#define SEC_TS_SID_GESTURE		0x14
#define SEC_TS_GESTURE_CODE_SPAY		0x00
#define SEC_TS_GESTURE_CODE_DOUBLE_TAP		0x01

#define SEC_TS_COORDINATE_ACTION_NONE		0
#define SEC_TS_COORDINATE_ACTION_PRESS		1
#define SEC_TS_COORDINATE_ACTION_MOVE		2
#define SEC_TS_COORDINATE_ACTION_RELEASE	3

#define SEC_TS_TOUCHTYPE_NORMAL		0
#define SEC_TS_TOUCHTYPE_HOVER		1
#define SEC_TS_TOUCHTYPE_FLIPCOVER	2
#define SEC_TS_TOUCHTYPE_GLOVE		3
#define SEC_TS_TOUCHTYPE_STYLUS		4
#define SEC_TS_TOUCHTYPE_PALM		5
#define SEC_TS_TOUCHTYPE_WET		6
#define SEC_TS_TOUCHTYPE_PROXIMITY	7
#define SEC_TS_TOUCHTYPE_JIG		8
#define SEC_TS_TOUCHTYPE_GRIP		10

/* SEC_TS_INFO : Info acknowledge event */
#define SEC_TS_ACK_BOOT_COMPLETE	0x00
#define SEC_TS_ACK_WET_MODE	0x1

/* SEC_TS_VENDOR_INFO : Vendor acknowledge event */
#define SEC_TS_VENDOR_ACK_OFFSET_CAL_DONE	0x40
#define SEC_TS_VENDOR_ACK_SELF_TEST_DONE	0x41
#define SEC_TS_VENDOR_ACK_P2P_TEST_DONE		0x42

/* SEC_TS_STATUS_EVENT_USER_INPUT */
#define SEC_TS_EVENT_FORCE_KEY	0x1

/* SEC_TS_STATUS_EVENT_CUSTOMLIB_INFO */
#define SEC_TS_EVENT_CUSTOMLIB_FORCE_KEY	0x00

/* SEC_TS_ERROR : Error event */
#define SEC_TS_ERR_EVNET_CORE_ERR	0x0
#define SEC_TS_ERR_EVENT_QUEUE_FULL	0x01
#define SEC_TS_ERR_EVENT_ESD		0x2

#define SEC_TS_BIT_SETFUNC_TOUCH		(1 << 0)
#define SEC_TS_BIT_SETFUNC_MUTUAL		(1 << 0)
#define SEC_TS_BIT_SETFUNC_HOVER		(1 << 1)
#define SEC_TS_BIT_SETFUNC_COVER		(1 << 2)
#define SEC_TS_BIT_SETFUNC_GLOVE		(1 << 3)
#define SEC_TS_BIT_SETFUNC_STYLUS		(1 << 4)
#define SEC_TS_BIT_SETFUNC_PALM			(1 << 5)
#define SEC_TS_BIT_SETFUNC_WET			(1 << 6)
#define SEC_TS_BIT_SETFUNC_PROXIMITY		(1 << 7)

#define SEC_TS_DEFAULT_ENABLE_BIT_SETFUNC	(SEC_TS_BIT_SETFUNC_TOUCH |\
						SEC_TS_BIT_SETFUNC_PALM |\
						SEC_TS_BIT_SETFUNC_WET)

#define SEC_TS_BIT_CHARGER_MODE_NO			(0x1 << 0)
#define SEC_TS_BIT_CHARGER_MODE_WIRE_CHARGER		(0x1 << 1)
#define SEC_TS_BIT_CHARGER_MODE_WIRELESS_CHARGER	(0x1 << 2)
#define SEC_TS_BIT_CHARGER_MODE_WIRELESS_BATTERY_PACK	(0x1 << 3)

#ifdef PAT_CONTROL
/*
 *  <<< apply to server >>>
 *  0x00 : no action
 *  0x01 : clear nv
 *  0x02 : pat magic
 *  0x03 : rfu
 *
 *  <<< use for temp bin >>>
 *  0x05 : forced clear nv & f/w update  before pat magic, eventhough same f/w
 *  0x06 : rfu
 **/
#define PAT_CONTROL_NONE			0x00
#define PAT_CONTROL_CLEAR_NV		0x01
#define PAT_CONTROL_PAT_MAGIC		0x02
#define PAT_CONTROL_FORCE_UPDATE	0x05

#define PAT_COUNT_ZERO			0x00
#define PAT_MAX_LCIA			0x80
#define PAT_MAGIC_NUMBER		0x83
#define PAT_MAX_MAGIC			0xC5
#define PAT_EXT_FACT			0xE0
#define PAT_MAX_EXT			0xF5
#endif

#define STATE_MANAGE_ON			1
#define STATE_MANAGE_OFF		0

#define SEC_TS_STATUS_NOT_CALIBRATION	0x50
#define SEC_TS_STATUS_CALIBRATION_SDC	0xA1
#define SEC_TS_STATUS_CALIBRATION_SEC	0xA2

#define SEC_TS_CMD_EDGE_HANDLER		0xAA
#define SEC_TS_CMD_EDGE_AREA		0xAB
#define SEC_TS_CMD_DEAD_ZONE		0xAC
#define SEC_TS_CMD_LANDSCAPE_MODE	0xAD

enum spec_check_type {
	SPEC_NO_CHECK			= 0,
	SPEC_CHECK			= 1,
	SPEC_PASS			= 2,
	SPEC_FAIL			= 3,
};

enum region_type {
	REGION_NORMAL			= 0,
	REGION_EDGE			= 1,
	REGION_CORNER			= 2,
	REGION_NOTCH			= 3,
	REGION_TYPE_COUNT		= 4,
	/* REGION type should be continuous number start from 0,
	 * since REGION_TYPE_COUNT is used for type count
	 */
};

enum grip_write_mode {
	G_NONE				= 0,
	G_SET_EDGE_HANDLER		= 1,
	G_SET_EDGE_ZONE			= 2,
	G_SET_NORMAL_MODE		= 4,
	G_SET_LANDSCAPE_MODE	= 8,
	G_CLR_LANDSCAPE_MODE	= 16,
};
enum grip_set_data {
	ONLY_EDGE_HANDLER		= 0,
	GRIP_ALL_DATA			= 1,
};

enum TOUCH_POWER_MODE {
	SEC_TS_STATE_POWER_OFF = 0,
	SEC_TS_STATE_SUSPEND,
	SEC_TS_STATE_LPM,
	SEC_TS_STATE_POWER_ON
};

enum TOUCH_SYSTEM_MODE {
	TOUCH_SYSTEM_MODE_BOOT		= 0,
	TOUCH_SYSTEM_MODE_CALIBRATION	= 1,
	TOUCH_SYSTEM_MODE_TOUCH		= 2,
	TOUCH_SYSTEM_MODE_SELFTEST	= 3,
	TOUCH_SYSTEM_MODE_FLASH		= 4,
	TOUCH_SYSTEM_MODE_LOWPOWER	= 5,
	TOUCH_SYSTEM_MODE_SLEEP		= 6
};

enum TOUCH_MODE_STATE {
	TOUCH_MODE_STATE_IDLE		= 0,
	TOUCH_MODE_STATE_HOVER		= 1,
	TOUCH_MODE_STATE_STOP		= 1,
	TOUCH_MODE_STATE_TOUCH		= 2,
	TOUCH_MODE_STATE_NOISY		= 3,
	TOUCH_MODE_STATE_CAL		= 4,
	TOUCH_MODE_STATE_CAL2		= 5,
	TOUCH_MODE_STATE_WAKEUP		= 10
};

enum {
	TEST_OPEN			= (0x1 << 0),
	TEST_NODE_VARIANCE		= (0x1 << 1),
	TEST_SHORT			= (0x1 << 2),
	TEST_SELF_NODE			= (0x1 << 5),
	TEST_NOT_SAVE			= (0x1 << 7),
	TEST_HIGH_FREQ			= (0x1 << 8),
};

enum switch_system_mode {
	TO_TOUCH_MODE			= 0,
	TO_LOWPOWER_MODE		= 1,
	TO_SELFTEST_MODE		= 2,
	TO_FLASH_MODE			= 3,
};

enum noise_mode_param {
	NOISE_MODE_DEFALUT	= 0x00,
	NOISE_MODE_OFF		= 0x10,
	NOISE_MODE_FORCE_ON	= 0x11,
};

enum {
	TYPE_RAW_DATA			= 0,	/* Total - Offset : delta data
						 **/
	TYPE_SIGNAL_DATA		= 1,	/* Signal - Filtering &
						 * Normalization
						 **/
	TYPE_AMBIENT_BASELINE		= 2,	/* Cap Baseline
						 **/
	TYPE_AMBIENT_DATA		= 3,	/* Cap Ambient
						 **/
	TYPE_REMV_BASELINE_DATA		= 4,
	TYPE_DECODED_DATA		= 5,	/* Raw */
	TYPE_REMV_AMB_DATA		= 6,	/* TYPE_RAW_DATA -
						 * TYPE_AMBIENT_DATA
						 **/
	TYPE_NORM2_DATA			= 15,	/* After fs norm. data
						 **/
	TYPE_OFFSET_DATA_SEC		= 19,	/* Cap Offset in SEC
						 * Manufacturing Line
						 **/
	TYPE_OFFSET_DATA_SDC		= 29,	/* Cap Offset in SDC
						 * Manufacturing Line
						 **/
	TYPE_NOI_P2P_MIN		= 30,	/* Peak-to-peak noise Min
						 **/
	TYPE_NOI_P2P_MAX		= 31,	/* Peak-to-peak noise Max
						 **/
	TYPE_OFFSET_DATA_SDC_CM2	= 129,
	TYPE_OFFSET_DATA_SDC_NOT_SAVE	= 229,
	TYPE_INVALID_DATA		= 0xFF,	/* Invalid data type for
						 * release factory mode
						 **/
};

enum RESET_MODE {
	RESET_MODE_NA		= 0x00,
	RESET_MODE_SW		= 0x01,
	RESET_MODE_HW		= 0x02,
	RESET_MODE_AUTO		= (RESET_MODE_SW | RESET_MODE_HW),
};

enum CUSTOMLIB_EVENT_TYPE {
	CUSTOMLIB_EVENT_TYPE_SPAY			= 0x04,
	CUSTOMLIB_EVENT_TYPE_PRESSURE_TOUCHED		= 0x05,
	CUSTOMLIB_EVENT_TYPE_PRESSURE_RELEASED		= 0x06,
	CUSTOMLIB_EVENT_TYPE_AOD			= 0x08,
	CUSTOMLIB_EVENT_TYPE_AOD_PRESS			= 0x09,
	CUSTOMLIB_EVENT_TYPE_AOD_LONGPRESS		= 0x0A,
	CUSTOMLIB_EVENT_TYPE_AOD_DOUBLETAB		= 0x0B,
	CUSTOMLIB_EVENT_TYPE_AOD_HOMEKEY_PRESS		= 0x0C,
	CUSTOMLIB_EVENT_TYPE_AOD_HOMEKEY_RELEASE	= 0x0D,
	CUSTOMLIB_EVENT_TYPE_AOD_HOMEKEY_RLS_NO_HAPTIC	= 0x0E
};

enum {
	SEC_TS_BUS_REF_SCREEN_ON	= 0x01,
	SEC_TS_BUS_REF_IRQ		= 0x02,
	SEC_TS_BUS_REF_RESET		= 0x04,
	SEC_TS_BUS_REF_FW_UPDATE	= 0x08,
	SEC_TS_BUS_REF_INPUT_DEV	= 0x10,
	SEC_TS_BUS_REF_READ_INFO	= 0x20,
	SEC_TS_BUS_REF_SYSFS		= 0x40,
	SEC_TS_BUS_REF_FORCE_ACTIVE	= 0x80,
	SEC_TS_BUS_REF_BUGREPORT	= 0x100
};

enum {
	SEC_TS_ERR_NA = 0,
	SEC_TS_ERR_INIT,
	SEC_TS_ERR_ALLOC_FRAME,
	SEC_TS_ERR_ALLOC_FRAME_SS,
	SEC_TS_ERR_ALLOC_GAINTABLE,
	SEC_TS_ERR_REG_INPUT_DEV,
	SEC_TS_ERR_REG_INPUT_PAD_DEV
};

#define CMD_RESULT_WORD_LEN		10

#define SEC_TS_IO_RESET_CNT		3
#define SEC_TS_IO_RETRY_CNT		3
#define SEC_TS_WAIT_RETRY_CNT		10

#define SEC_TS_MODE_CUSTOMLIB_SPAY			(1 << 1)
#define SEC_TS_MODE_CUSTOMLIB_AOD			(1 << 2)
#define SEC_TS_MODE_CUSTOMLIB_FORCE_KEY	(1 << 6)

#define SEC_TS_MODE_LOWPOWER_FLAG   (SEC_TS_MODE_CUSTOMLIB_SPAY |\
				    SEC_TS_MODE_CUSTOMLIB_AOD |\
				    SEC_TS_MODE_CUSTOMLIB_FORCE_KEY)

#define SEC_TS_AOD_GESTURE_PRESS		(1 << 7)
#define SEC_TS_AOD_GESTURE_LONGPRESS		(1 << 6)
#define SEC_TS_AOD_GESTURE_DOUBLETAB		(1 << 5)

#define SEC_TS_CUSTOMLIB_EVENT_PRESSURE_TOUCHED		(1 << 6)
#define SEC_TS_CUSTOMLIB_EVENT_PRESSURE_RELEASED		(1 << 7)

enum sec_ts_cover_id {
	SEC_TS_FLIP_WALLET = 0,
	SEC_TS_VIEW_COVER,
	SEC_TS_COVER_NOTHING1,
	SEC_TS_VIEW_WIRELESS,
	SEC_TS_COVER_NOTHING2,
	SEC_TS_CHARGER_COVER,
	SEC_TS_VIEW_WALLET,
	SEC_TS_LED_COVER,
	SEC_TS_CLEAR_FLIP_COVER,
	SEC_TS_QWERTY_KEYBOARD_EUR,
	SEC_TS_QWERTY_KEYBOARD_KOR,
	SEC_TS_MONTBLANC_COVER = 100,
};

enum sec_fw_update_status {
	SEC_NOT_UPDATE = 0,
	SEC_NEED_FW_UPDATE,
	SEC_NEED_CALIBRATION_ONLY,
	SEC_NEED_FW_UPDATE_N_CALIBRATION,
};

enum tsp_hw_parameter {
	TSP_ITO_CHECK		= 1,
	TSP_RAW_CHECK		= 2,
	TSP_MULTI_COUNT		= 3,
	TSP_WET_MODE		= 4,
	TSP_COMM_ERR_COUNT	= 5,
	TSP_MODULE_ID		= 6,
};

enum {
	HEATMAP_OFF	= 0,
	HEATMAP_PARTIAL	= 1,
	HEATMAP_FULL	= 2
};

enum {
	GRIP_PRESCREEN_OFF	= 0,
	GRIP_PRESCREEN_MODE_1	= 1,
	GRIP_PRESCREEN_MODE_2	= 2,
	GRIP_PRESCREEN_MODE_3	= 3
};

enum {
	GRIP_PRESCREEN_TIMEOUT_MIN	= 0,
	GRIP_PRESCREEN_TIMEOUT_MAX	= 480
};

enum {
	ENCODED_ENABLE_OFF	= 0,
	ENCODED_ENABLE_ON	= 1
};

/* Motion filter finite state machine (FSM) states
 * SEC_TS_MF_FILTERED        - default coordinate filtering
 * SEC_TS_MF_UNFILTERED      - unfiltered single-touch coordinates
 * SEC_TS_MF_FILTERED_LOCKED - filtered coordinates. Locked until touch is
 *			       lifted.
 */
enum motion_filter_state_t {
	SEC_TS_MF_FILTERED         = 0,
	SEC_TS_MF_UNFILTERED       = 1,
	SEC_TS_MF_FILTERED_LOCKED  = 2
};

#if IS_ENABLED(CONFIG_TOUCHSCREEN_HEATMAP)
/* Local heatmap */
#define LOCAL_HEATMAP_WIDTH 7
#define LOCAL_HEATMAP_HEIGHT 7

struct heatmap_report {
	int8_t offset_x;
	uint8_t size_x;
	int8_t offset_y;
	uint8_t size_y;
	/* data is in BE order; order should be enforced after data is read */
	strength_t data[LOCAL_HEATMAP_WIDTH * LOCAL_HEATMAP_HEIGHT];
} __packed;

struct heatmap_data {
	ktime_t timestamp;
	uint16_t size_x;
	uint16_t size_y;
	uint8_t *data;
} __packed;
#endif

#define TEST_MODE_MIN_MAX		false
#define TEST_MODE_ALL_NODE		true
#define TEST_MODE_READ_FRAME		0
#define TEST_MODE_READ_CHANNEL		1
#define TEST_MODE_READ_ALL		2

/* factory test mode */
struct sec_ts_test_mode {
	u8 type;
	short min[REGION_TYPE_COUNT];
	short max[REGION_TYPE_COUNT];
	bool allnode;
	u8 frame_channel;
	enum spec_check_type spec_check;
};

struct sec_ts_fw_file {
	u8 *data;
	u32 pos;
	size_t size;
};

/*
 * write 0xE4 [ 11 | 10 | 01 | 00 ]
 * MSB <-------------------> LSB
 * read 0xE4
 * mapping sequnce : LSB -> MSB
 * struct sec_ts_test_result {
 * * assy : front + OCTA assay
 * * module : only OCTA
 *	 union {
 *		 struct {
 *			 u8 assy_count:2;		-> 00
 *			 u8 assy_result:2;		-> 01
 *			 u8 module_count:2;	-> 10
 *			 u8 module_result:2;	-> 11
 *		 } __attribute__ ((packed));
 *		 unsigned char data[1];
 *	 };
 *};
 */
struct sec_ts_test_result {
	union {
		struct {
			u8 assy_count:2;
			u8 assy_result:2;
			u8 module_count:2;
			u8 module_result:2;
		} __packed;
		unsigned char data[1];
	};
};

/* 8 byte */
struct sec_ts_gesture_status {
	u8 eid:2;
	u8 stype:4;
	u8 sf:2;
	u8 gesture_id;
	u8 gesture_data_1;
	u8 gesture_data_2;
	u8 gesture_data_3;
	u8 gesture_data_4;
	u8 reserved_1;
	u8 left_event_5_0:6;
	u8 reserved_2:2;
} __packed;


/* status id for sec_ts event */
#define SEC_TS_EVENT_STATUS_ID_HOPPING		0x33
#define SEC_TS_EVENT_STATUS_ID_REPORT_RATE	0x34
#define SEC_TS_EVENT_STATUS_ID_VSYNC		0x35
#define SEC_TS_EVENT_STATUS_ID_NOISE		0x64
#define SEC_TS_EVENT_STATUS_ID_WLC		0x66
#define SEC_TS_EVENT_STATUS_ID_GRIP		0x69
#define SEC_TS_EVENT_STATUS_ID_FOD		0x6B
#define SEC_TS_EVENT_STATUS_ID_PALM		0x70

/* 8 byte */
struct sec_ts_fod_event {
	struct {
		u8 type;
		u8 id;
		u8 status;
		u8 x_b7_b0;
		union {
			struct {
				u8 y_b11_b8:4;
				u8 x_b11_b8:4;
			};
			u8 x_y_b11_b8;
		};
		u8 y_b7_b0;
		u8 reserved[2];
	};
} __packed;

/* 8 byte */
struct sec_ts_event_status {
	union {
		struct {
			u8 eid:2;
			u8 stype:4;
			u8 sf:2;
			u8 status_id;
			u8 status_data_1;
			u8 status_data_2;
			u8 status_data_3;
			u8 status_data_4;
			u8 status_data_5;
			u8 left_event_5_0:6;
			u8 reserved_2:2;
		}  __packed;
		u8 data[8];
	};
} __packed;

/* 8 byte */
struct sec_ts_event_coordinate {
	u8 eid:2;
	u8 tid:4;
	u8 tchsta:2;
	u8 x_11_4;
	u8 y_11_4;
	u8 y_3_0:4;
	u8 x_3_0:4;
	u8 major;
	u8 minor;
	u8 z:6;
	u8 ttype_3_2:2;
	u8 left_event:6;
	u8 ttype_1_0:2;
} __packed;

/* 8 byte */
struct sec_ts_event_hopping {
	u8 eid:2;
	u8 stype:4;
	u8 sf:2;
	u8 event_id;
	u8 id:4;
	u8 cause: 4;
	u8 prev_id;
	u8 noise_lvl[2];
	u8 reserved_6;
	u8 reserved_7;
} __packed;

/* not fixed */
struct sec_ts_coordinate {
	u8 id;
	u8 ttype;
	u8 action;
	u16 x;
	u16 y;
	u8 z;
	u8 hover_flag;
	u8 glove_flag;
	u8 touch_height;
	u16 mcount;
	u16 major;
	u16 minor;
	bool palm;
	u8 left_event;
	bool grip;
	/* for debug purpose. */
	u16 x_pressed;	/* x coord on first down timing. */
	u16 y_pressed;	/* y coord on first down timing. */
	ktime_t ktime_pressed;
	ktime_t ktime_released;
};

struct sec_ts_health_check {
	ktime_t int_ktime;
	u64 int_idx;
	u64 coord_idx;
	u64 status_idx;
	/* Slot active bit from FW. */
	unsigned long active_bit;
	/* Check whether have coord, status or unknown event. */
	bool coord_updated;
	bool status_updated;
};

struct sec_ts_data {
	u32 isr_pin;

	u32 crc_addr;
	u32 fw_addr;
	u32 para_addr;
	u32 flash_page_size;
	u8 boot_ver[3];

	struct device *dev;
#ifdef I2C_INTERFACE
	struct i2c_client *client;
#else
	struct spi_device *client;
#endif
	struct input_dev *input_dev;
	struct input_dev *input_dev_pad;
	struct input_dev *input_dev_touch;
	struct sec_ts_plat_data *plat_data;
	struct sec_ts_coordinate coord[MAX_SUPPORT_TOUCH_COUNT +
					MAX_SUPPORT_HOVER_COUNT];

	ktime_t timestamp; /* time that the event was first received from
			    * the touch IC, acquired during hard interrupt,
			    * in CLOCK_MONOTONIC
			    **/

	s64 longest_duration; /* ms unit */

	u8 lowpower_mode;
	u8 lowpower_status;
	u8 dex_mode;
	char *dex_name;
	u8 brush_mode;
	u8 touchable_area;
	volatile bool input_closed;

	struct mutex bus_mutex;
	u16 bus_refmask;
	struct completion bus_resumed;
	struct completion boot_completed;

	unsigned int touch_count;	/* active touch slot(s). */
	int tx_count;
	int rx_count;
	int io_burstmax;
	int ta_status;
	volatile int power_status;
	int raw_status;
	int touchkey_glove_mode_status;
	u16 touch_functions;
	u8 charger_mode;
	struct sec_ts_event_coordinate touchtype;
	u8 gesture_status[6];
	u8 cal_status;
	struct mutex lock;
	struct mutex device_mutex;
	struct mutex io_mutex;
	struct mutex eventlock;

	struct drm_bridge panel_bridge;
	struct drm_connector *connector;
	bool is_panel_lp_mode;
	int display_refresh_rate;	/* Display rate in Hz */

	struct pm_qos_request pm_qos_req;

	/* Stop changing charger mode by notifier */
	u8 ignore_charger_nb;
	/* Stop changing motion filter and keep fw design */
	u8 use_default_mf;
	/* Motion filter finite state machine (FSM) state */
	enum motion_filter_state_t mf_state;
	/* Time of initial single-finger touch down. This timestamp is used to
	 * compute the duration a single finger is touched before it is lifted.
	 */
	ktime_t mf_downtime;

	u8 print_format;
	u8 ms_frame_type;
	u8 ss_frame_type;
	int heatmap_dump;
#if IS_ENABLED(CONFIG_TOUCHSCREEN_HEATMAP)
	struct v4l2_heatmap v4l2;
	struct heatmap_data mutual_strength_heatmap;
	strength_t *heatmap_buff;
	strength_t *encoded_buff;
	bool heatmap_init_done;
	bool v4l2_mutual_strength_updated;
#endif

#if IS_ENABLED(CONFIG_TOUCHSCREEN_OFFLOAD)
	struct touch_offload_context offload;
#endif

#ifdef USE_POWER_RESET_WORK
	struct delayed_work reset_work;
	volatile bool reset_is_on_going;
#endif

#ifdef SEC_TS_FW_UPDATE_ON_PROBE
	struct work_struct fw_update_work;
#else
	struct delayed_work fw_update_work;
	struct workqueue_struct *fw_update_wq;
#endif

#ifdef USE_CHARGER_WORK
	struct work_struct charger_work;	/* charger work */
#endif
	struct work_struct suspend_work;
	struct work_struct resume_work;
	struct workqueue_struct *event_wq;	/* Used for event handler,
						 * suspend, resume threads
						 **/
	struct completion resume_done;
	struct sec_cmd_data sec;
	union {
		short *pFrame;
		short *pFrameMS;
	};

	/* only available if sec_ts_read_frame_and_channel() be called */
	short *pFrameSS;

#ifdef USE_STIM_PAD
	u8 *gainTable;
#endif

	bool probe_done;
	bool reinit_done;
	bool flip_enable;
	int cover_type;
	u8 cover_cmd;
	u16 rect_data[4];

	int tspid_val;
	int tspicid_val;

	bool use_customlib;
	unsigned int scrub_id;
	unsigned int scrub_x;
	unsigned int scrub_y;

	u8 grip_edgehandler_direction;
	int grip_edgehandler_start_y;
	int grip_edgehandler_end_y;
	u16 grip_edge_range;
	u8 grip_deadzone_up_x;
	u8 grip_deadzone_dn_x;
	int grip_deadzone_y;
	u8 grip_landscape_mode;
	int grip_landscape_edge;
	u16 grip_landscape_deadzone;

#ifdef CONFIG_TOUCHSCREEN_DUMP_MODE
	struct delayed_work ghost_check;
#endif
	u8 tsp_dump_lock;

	int nv;
	int cal_count;
	int tune_fix_ver;
	bool external_factory;

	int report_rate;
	int vsync;
	int wet_mode;

	unsigned char ito_test[4];		/* ito panel tx/rx chanel */
	unsigned char check_multi;
	unsigned int multi_count;		/* multi touch count */
	unsigned int palm_count;
	unsigned int wet_count;			/* wet mode count */
	unsigned int dive_count;		/* dive mode count */
	unsigned int comm_err_count;		/* comm error count */
	unsigned int io_err_count;		/* io error count */
	unsigned int hw_reset_count;
	/*
	 * accumulated count of pressed
	 * touch from resume to suspend.
	 */
	unsigned int pressed_count;
	unsigned int checksum_result;		/* checksum result */
	unsigned char module_id[4];
	unsigned int all_finger_count;
	unsigned int all_force_count;
	unsigned int all_aod_tap_count;
	unsigned int all_spay_count;
	unsigned int max_z_value;
	unsigned int min_z_value;
	unsigned int sum_z_value;
	unsigned char pressure_cal_base;
	unsigned char pressure_cal_delta;

#ifdef USE_PRESSURE_SENSOR
	short pressure_left;
	short pressure_center;
	short pressure_right;
	u8 pressure_user_level;
#endif
	union {
		u32 debug;
		struct {
		u32 debug_events : 1;
		u32 debug_status : 1;
		u32 debug_reserved : 30;
		};
	};

	int fs_postcal_mean;

	bool is_fw_corrupted;
	union {
		u8 cali_report[8];
		struct {
		u8 cali_report_try_cnt;
		u8 cali_report_pass_cnt;
		u8 cali_report_fail_cnt;
		u8 cali_report_status;
		u8 cali_report_param_ver[4];
		};
	};

	/* slot id active state(bit mask) for grip/palm
	 **/
	unsigned long tid_palm_state;
	unsigned long tid_grip_state;
	/* slot id active state(bit mask) for all touch types
	 **/
	unsigned long tid_touch_state;
	/* Record the state that grip/palm was leaved once ever after any
	 * touch pressed. This state will set to default after all active
	 * touch released.
	 **/
	bool palms_leaved_once;
	bool grips_leaved_once;

#if IS_ENABLED(CONFIG_TOUCHSCREEN_TBN)
	u32 tbn_register_mask;
#endif

	struct power_supply *wireless_psy;
	struct power_supply *usb_psy;
	struct notifier_block psy_nb;
	bool wlc_online;
	bool usb_present;
	bool force_wlc;
	ktime_t usb_changed_ktime;
	ktime_t wlc_changed_ktime;

	ktime_t bugreport_ktime_start;
	ktime_t ktime_resume;
	u64 int_cnt;
	u64 coord_event_cnt;
	u64 status_event_cnt;

	int (*sec_ts_write)(struct sec_ts_data *ts, u8 reg,
				u8 *data, int len);

	int (*sec_ts_read)(struct sec_ts_data *ts, u8 reg,
				    u8 *data, int len);
	int (*sec_ts_read_heap)(struct sec_ts_data *ts, u8 reg,
				    u8 *data, int len);

	int (*sec_ts_write_burst)(struct sec_ts_data *ts,
					   u8 *data, int len);
	int (*sec_ts_write_burst_heap)(struct sec_ts_data *ts,
					   u8 *data, int len);

	int (*sec_ts_read_bulk)(struct sec_ts_data *ts,
					 u8 *data, int len);
	int (*sec_ts_read_bulk_heap)(struct sec_ts_data *ts,
					 u8 *data, int len);

	int (*sec_ts_read_customlib)(struct sec_ts_data *ts,
				     u8 *data, int len);

	/* alloc for io read buffer */
	u8 io_read_buf[IO_PREALLOC_READ_BUF_SZ];
	/* alloc for io write buffer */
	u8 io_write_buf[IO_PREALLOC_WRITE_BUF_SZ];
};

struct sec_ts_plat_data {
	int fod_x;
	int fod_y;
	int max_x;
	int max_y;
	unsigned int irq_gpio;
	int irq_type;
	int io_burstmax;
	int always_lpmode;
	int bringup;
	int mis_cal_check;
	int heatmap_mode;
	int grip_prescreen_mode;
	int grip_prescreen_timeout;
	bool is_heatmap_enabled;
	int encoded_enable;
	int encoded_frame_counter;
	int encoded_skip_counter;
#ifdef PAT_CONTROL
	int pat_function;
	int afe_base;
#endif
	const char *firmware_name;
	const char *model_name;
	const char *project_name;
	struct regulator *regulator_vdd;
	struct regulator *regulator_avdd;

	u32 panel_revision;
	u8 core_version_of_ic[4];
	u8 core_version_of_bin[4];
	u8 config_version_of_ic[4];
	u8 config_version_of_bin[4];
	u8 img_version_of_ic[4];
	u8 img_version_of_bin[4];

	struct pinctrl *pinctrl;

	int (*power)(void *data, bool on);
	void (*enable_sync)(bool on);
	int tsp_icid;
	int tsp_id;
	int vsync_gpio;
	int switch_gpio;
	int reset_gpio;

	bool regulator_boot_on;
	bool support_mt_pressure;
	bool support_dex;
	bool support_sidegesture;

	struct drm_panel *panel;
	u32 initial_panel_index;
	u32 offload_id;

	/* convert mm to pixel for major and minor */
	u8 mm2px;
};

void sec_ts_hc_dump(struct sec_ts_data *ts);
void sec_ts_debug_dump(struct sec_ts_data *ts);
int sec_ts_stop_device(struct sec_ts_data *ts);
int sec_ts_start_device(struct sec_ts_data *ts);
int sec_ts_hw_reset(struct sec_ts_data *ts, bool wait_for_done);
int sec_ts_sw_reset(struct sec_ts_data *ts, bool wait_for_done);
int sec_ts_system_reset(struct sec_ts_data *ts,
			enum RESET_MODE mode,
			bool wait_for_done,
			bool sense_on);
int sec_ts_set_lowpowermode(struct sec_ts_data *ts, u8 mode);
int sec_ts_firmware_update_on_probe(struct sec_ts_data *ts, bool force_update);
int sec_ts_firmware_update_on_hidden_menu(struct sec_ts_data *ts,
					    int update_type);
int sec_ts_glove_mode_enables(struct sec_ts_data *ts, int mode);
int sec_ts_set_cover_type(struct sec_ts_data *ts, bool enable);
int sec_ts_wait_for_ready(struct sec_ts_data *ts, unsigned int ack);
int sec_ts_wait_for_ready_with_count(struct sec_ts_data *ts, unsigned int ack,
				     unsigned int count);
int sec_ts_try_wake(struct sec_ts_data *ts, bool wake_setting);
int sec_ts_set_bus_ref(struct sec_ts_data *ts, u16 ref, bool enable);

int sec_ts_function(int (*func_init)(void *device_data),
		    void (*func_remove)(void));
int sec_ts_fn_init(struct sec_ts_data *ts);
int sec_ts_read_calibration_report(struct sec_ts_data *ts);
int sec_ts_execute_force_calibration(struct sec_ts_data *ts, int cal_mode);
int sec_ts_fix_tmode(struct sec_ts_data *ts, u8 mode, u8 state);
int sec_ts_release_tmode(struct sec_ts_data *ts);
int get_tsp_nvm_data(struct sec_ts_data *ts, u8 offset);
void set_tsp_nvm_data_clear(struct sec_ts_data *ts, u8 offset);
#ifdef SEC_TS_SUPPORT_CUSTOMLIB
int sec_ts_set_custom_library(struct sec_ts_data *ts);
int sec_ts_check_custom_library(struct sec_ts_data *ts);
#endif
void sec_ts_unlocked_release_all_finger(struct sec_ts_data *ts);
void sec_ts_locked_release_all_finger(struct sec_ts_data *ts);
void sec_ts_fn_remove(struct sec_ts_data *ts);
void sec_ts_delay(unsigned int ms);
int sec_ts_read_information(struct sec_ts_data *ts);
#ifdef PAT_CONTROL
void set_pat_magic_number(struct sec_ts_data *ts);
#endif
int sec_ts_run_rawdata_type(struct sec_ts_data *ts, struct sec_cmd_data *sec);
void sec_ts_run_rawdata_all(struct sec_ts_data *ts, bool full_read);
int execute_selftest(struct sec_ts_data *ts, u32 option);
int execute_p2ptest(struct sec_ts_data *ts);
int sec_ts_read_raw_data(struct sec_ts_data *ts,
		struct sec_cmd_data *sec, struct sec_ts_test_mode *mode);
u8 sec_ts_run_cal_check(struct sec_ts_data *ts);

#if (1)//!defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
int sec_ts_raw_device_init(struct sec_ts_data *ts);
#endif
void sec_ts_raw_device_exit(struct sec_ts_data *ts);

extern struct class *sec_class;

#if defined(CONFIG_FB_MSM_MDSS_SAMSUNG)
extern int get_lcd_attached(char *mode);
#endif

#if defined(CONFIG_EXYNOS_DECON_FB)
extern int get_lcd_info(char *arg);
#endif

#ifdef CONFIG_MOTOR_DRV_MAX77865
extern int haptic_homekey_press(void);
extern int haptic_homekey_release(void);
#else
#define haptic_homekey_press() {}
#define haptic_homekey_release() {}
#endif

extern bool tsp_init_done;

extern struct sec_ts_data *ts_dup;

#ifdef CONFIG_BATTERY_SAMSUNG
extern unsigned int lpcharge;
#endif

extern void set_grip_data_to_ic(struct sec_ts_data *ts, u8 flag);
extern void sec_ts_set_grip_type(struct sec_ts_data *ts, u8 set_type);


#endif
