/*
 * =====================================================================================
 *
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 *         Filename:    ve_interface.h
 *
 *      Description:
 *
 *          Version:    1.0
 *          Created:    2020-04-15
 *         Revision:    none
 *         Compiler:    gcc
 *
 *           Author:    jilinglin, <jilinglin@allwinnertech.com>
 *          Company:
 *
 * =====================================================================================
 */

#ifndef VE_INTERFACE_H
#define VE_INTERFACE_H

enum VE_REGISTER_GROUP {
	REG_GROUP_VETOP             = 0,
	REG_GROUP_MPEG_DECODER      = 1,
	REG_GROUP_H264_DECODER      = 2,
	REG_GROUP_VC1_DECODER       = 3,
	REG_GROUP_RV_DECODER        = 4,
	REG_GROUP_H265_DECODER      = 5,
	REG_GROUP_JPEG_DECODER      = 6,
	REG_GROUP_AVS_DECODER       = 7,
	REG_GROUP_VP9_DECODER       = 8,
	REG_GROUP_AVS2_DECODER      = 9,
};


enum VE_TYPE {
	VE_TYPE_AW = 0,
	VE_TYPE_GOOGLE,
};

enum CODEC_TYPE {
	VIDEO_CODEC_H264,
	VIDEO_CODEC_VP8,
	VIDEO_CODEC_AVS,
	VIDEO_CODEC_WMV3,
	VIDEO_CODEC_RX,
	VIDEO_CODEC_H265,
	VIDEO_CODEC_MJPEG,
	VIDEO_CODEC_AVS2,
	VIDEO_CODEC_VP9,
};

enum DRAMTYPE {
	DDRTYPE_DDR1_16BITS = 0,
	DDRTYPE_DDR1_32BITS = 1,
	DDRTYPE_DDR2_16BITS = 2,
	DDRTYPE_DDR2_32BITS = 3,
	DDRTYPE_DDR3_16BITS = 4,
	DDRTYPE_DDR3_32BITS = 5,
	DDRTYPE_DDR3_64BITS = 6,

	DDRTYPE_MIN = DDRTYPE_DDR1_16BITS,
	DDRTYPE_MAX = DDRTYPE_DDR3_64BITS,
};

enum RESET_VE_MODE {
	RESET_VE_NORMAL = 0,
	RESET_VE_SPECIAL = 1,  // for dtmb, we should reset ve not reset decode
};

enum VE_WORK_MODE {
	VE_NORMAL_MODE = 0,
	VE_DEC_MODE = 1,
	VE_ENC_MODE = 2,
	VE_JPG_DEC_MODE = 3,
};

struct ve_config {
	int dec_flag;
	int enc_flag;
	int codec_format;
	int just_isp_enable;
};

struct ve_interface {
	void (*reset)(struct ve_interface *);
	int  (*wait_interrupt)(struct ve_interface *);

	void (*set_ddr_mode)(struct ve_interface *, int);
	int (*set_ve_freq)(struct ve_interface *, int);
	void *(*get_reg_group_addr)(struct ve_interface *, int);
	unsigned long long (*get_ic_version)(struct ve_interface *);
	unsigned int (*get_phy_offset)(struct ve_interface *);

	void (*enable_ve)(struct ve_interface *);
	void (*disable_ve)(struct ve_interface *);
};

static inline void ve_reset(struct ve_interface *p)
{
	p->reset(p);
}

static inline int ve_wait_interrupt(struct ve_interface *p)
{
	return p->wait_interrupt(p);
}

static inline void *ve_get_group_reg_addr(struct ve_interface *p, int id)
{
	if (p && p->get_reg_group_addr)
		return p->get_reg_group_addr(p, id);
}

static inline void ve_set_ddr_mode(struct ve_interface *p, int ddr_mode)
{
	p->set_ddr_mode(p, ddr_mode);
}

static inline unsigned long long ve_get_ic_version(struct ve_interface *p)
{
	return p->get_ic_version(p);
}

static inline unsigned int ve_get_phy_offset(struct ve_interface *p)
{
	return p->get_phy_offset(p);
}

static inline void ve_enable_ve(struct ve_interface *p)
{
	p->enable_ve(p);
}

static inline void ve_disable_ve(struct ve_interface *p)
{
	p->disable_ve(p);
}

static inline int ve_set_ve_freq(struct ve_interface *p, int freq)
{
	return p->set_ve_freq(p, freq);
}

#if 0
static inline void ve_lock(struct ve_interface *p)
{
	p->lock(p);
}

static inline void ve_unlock(struct ve_interface *p)
{
	p->unlock(p);
}
#endif

struct ve_interface *ve_create(int type, struct ve_config config);
void ve_destory(int type, struct ve_interface *p);

#endif
