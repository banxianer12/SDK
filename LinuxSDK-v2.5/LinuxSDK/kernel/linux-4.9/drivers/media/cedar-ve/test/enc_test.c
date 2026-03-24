/*
 * =====================================================================================
 *
 * Filename: enc_test.c
 *
 *  Description:
 *
 *  Version: 1.0
 *  Created: 2020年04月23日 15时34分54秒
 *  Revision: none
 *  Compiler: gcc
 *
 *  Author: YOUR NAME (),
 *  Company:
 *
 * =====================================================================================
 */

#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include "vencoder.h"
#include "log.h"

#define ALIGN_XXB(y, x) (((x) + ((y)-1)) & ~((y)-1))
#define ROI_NUM 4
//no mutex
struct test_buf_info {
	int buf_len;
	char *buf;
	int data_len;
	int read_pos;
	int write_pos;
};

struct test_context {
	struct test_buf_info in_buf;
	struct test_buf_info out_buf;
	VideoEncoder *venc;

};

typedef struct {
	VencHeaderData          sps_pps_data;
	VencH264Param           h264Param;
	VencMBModeCtrl          h264MBMode;
	VencMBInfo              MBInfo;
	VencFixQP               fixQP;
	VencSuperFrameConfig    sSuperFrameCfg;
	VencH264SVCSkip         SVCSkip; // set SVC and skip_frame
	VencH264AspectRatio     sAspectRatio;
	VencH264VideoSignal     sVideoSignal;
	VencCyclicIntraRefresh  sIntraRefresh;
	VencROIConfig           sRoiConfig[ROI_NUM];
	VeProcSet               sVeProcInfo;
	VencOverlayInfoS        sOverlayInfo;
	VencSmartFun            sH264Smart;
} h264_func_t;

typedef struct {
	char			 intput_file[256];
	char			 output_file[256];
	char			 reference_file[256];
	char			 overlay_file[256];
	char			 log_file[256];
	int				 compare_flag;
	int				 log_flag;
	int				 compare_result;

	unsigned int  encode_frame_num;
	unsigned int  encode_format;

	unsigned int src_size;
	unsigned int dst_size;

	unsigned int src_width;
	unsigned int src_height;
	unsigned int dst_width;
	unsigned int dst_height;

	int frequency;
	int bit_rate;
	int frame_rate;
	int maxKeyFrame;
	unsigned int test_cycle;
	unsigned int test_overlay_flag;
	unsigned int test_lbc;
	unsigned int test_afbc;
	unsigned int limit_encode_speed; //* limit encoder speed by framerate
	unsigned int qp_min;
	unsigned int qp_max;

	h264_func_t h264_func;
	unsigned int nChannel;
} encode_param_t;

#define VE_LOGD(fmt, arg...) printk(KERN_DEBUG"VE: "fmt"\n", ##arg)
#define VE_LOGI(fmt, arg...) printk(KERN_INFO"VE: "fmt"\n", ##arg)
#define VE_LOGW(fmt, arg...) printk(KERN_WARNING"VE: "fmt"\n", ##arg)
#define VE_LOGE(fmt, arg...) printk(KERN_ERR"VE: "fmt"\n", ##arg)

#define TEST_IN_BUF_SIZE (1280 * 736 * 3 / 2)
#define TEST_OUT_BUF_SIZE (2 * 1024 * 1024)

static struct dentry *vedir;
static struct test_context *context;

void init_mb_mode(VencMBModeCtrl *pMBMode, encode_param_t *encode_param)
{
	unsigned int mb_num;
	//unsigned int j;

	mb_num = (ALIGN_XXB(16, encode_param->dst_width) >> 4)
				* (ALIGN_XXB(16, encode_param->dst_height) >> 4);
	pMBMode->p_info = kmalloc(sizeof(VencMBModeCtrlInfo) * mb_num, GFP_KERNEL);
	pMBMode->mode_ctrl_en = 1;

}

void init_mb_info(VencMBInfo *MBInfo, encode_param_t *encode_param)
{
	if (encode_param->encode_format == VENC_CODEC_H265) {
		MBInfo->num_mb = (ALIGN_XXB(32, encode_param->dst_width) *
							ALIGN_XXB(32, encode_param->dst_height)) >> 10;
	} else {
		MBInfo->num_mb = (ALIGN_XXB(16, encode_param->dst_width) *
							ALIGN_XXB(16, encode_param->dst_height)) >> 8;
	}
	MBInfo->p_para = (VencMBInfoPara *)kmalloc(sizeof(VencMBInfoPara) * MBInfo->num_mb, GFP_KERNEL);
	if (MBInfo->p_para == NULL) {
		loge("malloc MBInfo->p_para error\n");
		return;
	}
	logv("mb_num:%d, mb_info_queue_addr:%p\n", MBInfo->num_mb, MBInfo->p_para);
}


void init_fix_qp(VencFixQP *fixQP)
{
	fixQP->bEnable = 1;
	fixQP->nIQp = 35;
	fixQP->nPQp = 35;
}

void init_super_frame_cfg(VencSuperFrameConfig *sSuperFrameCfg)
{
	sSuperFrameCfg->eSuperFrameMode = VENC_SUPERFRAME_NONE;
	sSuperFrameCfg->nMaxIFrameBits = 30000*8;
	sSuperFrameCfg->nMaxPFrameBits = 15000*8;
}

void init_svc_skip(VencH264SVCSkip *SVCSkip)
{
	SVCSkip->nTemporalSVC = T_LAYER_4;
	switch (SVCSkip->nTemporalSVC) {
	case T_LAYER_4:
		SVCSkip->nSkipFrame = SKIP_8;
		break;
	case T_LAYER_3:
		SVCSkip->nSkipFrame = SKIP_4;
		break;
	case T_LAYER_2:
		SVCSkip->nSkipFrame = SKIP_2;
		break;
	default:
		SVCSkip->nSkipFrame = NO_SKIP;
		break;
	}
}

void init_aspect_ratio(VencH264AspectRatio *sAspectRatio)
{
	sAspectRatio->aspect_ratio_idc = 255;
	sAspectRatio->sar_width = 4;
	sAspectRatio->sar_height = 3;
}

void init_video_signal(VencH264VideoSignal *sVideoSignal)
{
	sVideoSignal->video_format = 5;
	sVideoSignal->src_colour_primaries = 0;
	sVideoSignal->dst_colour_primaries = 1;
}

void init_intra_refresh(VencCyclicIntraRefresh *sIntraRefresh)
{
	sIntraRefresh->bEnable = 1;
	sIntraRefresh->nBlockNumber = 10;
}

void init_roi(VencROIConfig *sRoiConfig)
{
	sRoiConfig[0].bEnable = 1;
	sRoiConfig[0].index = 0;
	sRoiConfig[0].nQPoffset = 10;
	sRoiConfig[0].sRect.nLeft = 0;
	sRoiConfig[0].sRect.nTop = 0;
	sRoiConfig[0].sRect.nWidth = 1280;
	sRoiConfig[0].sRect.nHeight = 320;

	sRoiConfig[1].bEnable = 1;
	sRoiConfig[1].index = 1;
	sRoiConfig[1].nQPoffset = 10;
	sRoiConfig[1].sRect.nLeft = 320;
	sRoiConfig[1].sRect.nTop = 180;
	sRoiConfig[1].sRect.nWidth = 320;
	sRoiConfig[1].sRect.nHeight = 180;

	sRoiConfig[2].bEnable = 1;
	sRoiConfig[2].index = 2;
	sRoiConfig[2].nQPoffset = 10;
	sRoiConfig[2].sRect.nLeft = 320;
	sRoiConfig[2].sRect.nTop = 180;
	sRoiConfig[2].sRect.nWidth = 320;
	sRoiConfig[2].sRect.nHeight = 180;

	sRoiConfig[3].bEnable = 1;
	sRoiConfig[3].index = 3;
	sRoiConfig[3].nQPoffset = 10;
	sRoiConfig[3].sRect.nLeft = 320;
	sRoiConfig[3].sRect.nTop = 180;
	sRoiConfig[3].sRect.nWidth = 320;
	sRoiConfig[3].sRect.nHeight = 180;
}

void init_alter_frame_rate_info(VencAlterFrameRateInfo *pAlterFrameRateInfo)
{
	memset(pAlterFrameRateInfo, 0, sizeof(VencAlterFrameRateInfo));
	pAlterFrameRateInfo->bEnable = 1;
	pAlterFrameRateInfo->bUseUserSetRoiInfo = 1;
	pAlterFrameRateInfo->sRoiBgFrameRate.nSrcFrameRate = 25;
	pAlterFrameRateInfo->sRoiBgFrameRate.nDstFrameRate = 5;

	pAlterFrameRateInfo->roi_param[0].bEnable = 1;
	pAlterFrameRateInfo->roi_param[0].index = 0;
	pAlterFrameRateInfo->roi_param[0].nQPoffset = 10;
	pAlterFrameRateInfo->roi_param[0].roi_abs_flag = 1;
	pAlterFrameRateInfo->roi_param[0].sRect.nLeft = 0;
	pAlterFrameRateInfo->roi_param[0].sRect.nTop = 0;
	pAlterFrameRateInfo->roi_param[0].sRect.nWidth = 320;
	pAlterFrameRateInfo->roi_param[0].sRect.nHeight = 320;

	pAlterFrameRateInfo->roi_param[1].bEnable = 1;
	pAlterFrameRateInfo->roi_param[1].index = 0;
	pAlterFrameRateInfo->roi_param[1].nQPoffset = 10;
	pAlterFrameRateInfo->roi_param[1].roi_abs_flag = 1;
	pAlterFrameRateInfo->roi_param[1].sRect.nLeft = 320;
	pAlterFrameRateInfo->roi_param[1].sRect.nTop = 320;
	pAlterFrameRateInfo->roi_param[1].sRect.nWidth = 320;
	pAlterFrameRateInfo->roi_param[1].sRect.nHeight = 320;
}

void init_enc_proc_info(VeProcSet *ve_proc_set)
{
	ve_proc_set->bProcEnable = 1;
	ve_proc_set->nProcFreq = 60;
}
/*
void init_overlay_info(VencOverlayInfoS *pOverlayInfo, encode_param_t *encode_param)
{
	int i;
	unsigned char num_bitMap = 2;
	BitMapInfoS* pBitMapInfo;
	unsigned int time_id_list[19];
	unsigned int start_mb_x;
	unsigned int start_mb_y;

	memset(pOverlayInfo, 0, sizeof(VencOverlayInfoS));

		FILE* icon_hdle = NULL;
		int width  = 464;
		int height = 32;
		memset(time_id_list, 0, sizeof(time_id_list));

		icon_hdle = fopen(encode_param->overlay_file, "r");
		logd("icon_hdle = %p", icon_hdle);
		if (icon_hdle == NULL) {
			printf("get icon_hdle error\n");
			return;
		}

		for (i = 0; i < num_bitMap; i++) {
			bit_map_info[i].argb_addr = NULL;
			bit_map_info[i].width = width;
			bit_map_info[i].height = height;

			bit_map_info[i].width_aligh16 = ALIGN_XXB(16, bit_map_info[i].width);
			bit_map_info[i].height_aligh16 = ALIGN_XXB(16, bit_map_info[i].height);
			if (bit_map_info[i].argb_addr == NULL) {
				bit_map_info[i].argb_addr =
			(unsigned char*)malloc(bit_map_info[i].width_aligh16*bit_map_info[i].height_aligh16*4);

				if (bit_map_info[i].argb_addr == NULL) {
					loge("malloc bit_map_info[%d].argb_addr fail\n", i);
					if (icon_hdle) {
						fclose(icon_hdle);
						icon_hdle = NULL;
					}

					return;
				}
			}
			logd("bitMap[%d] size[%d, %d], size_align16[%d, %d], argb_addr:%p\n", i,
														bit_map_info[i].width,
														bit_map_info[i].height,
														bit_map_info[i].width_aligh16,
														bit_map_info[i].height_aligh16,
														bit_map_info[i].argb_addr);

			int ret;
			ret = fread(bit_map_info[i].argb_addr, 1,
						bit_map_info[i].width*bit_map_info[i].height*4, icon_hdle);
			if (ret != (int)(bit_map_info[i].width*bit_map_info[i].height*4))
			loge("read bitMap[%d] error, ret value:%d\n", i, ret);

			bit_map_info[i].size
				= bit_map_info[i].width_aligh16 * bit_map_info[i].height_aligh16 * 4;
			fseek(icon_hdle, 0, SEEK_SET);
		}
		if (icon_hdle) {
			fclose(icon_hdle);
			icon_hdle = NULL;
		}
		pOverlayInfo->argb_type = VENC_OVERLAY_ARGB8888;
		pOverlayInfo->blk_num = num_bitMap;
		logd("blk_num:%d, argb_type:%d\n", pOverlayInfo->blk_num, pOverlayInfo->argb_type);
		//pOverlayInfo->invert_threshold = 200;
		//pOverlayInfo->invert_mode = 3;

		start_mb_x = 0;
		start_mb_y = 0;
		for (i=0; i<pOverlayInfo->blk_num; i++) {
			//id = time_id_list[i];
			//pBitMapInfo = &bit_map_info[id];
			pBitMapInfo = &bit_map_info[i];

			pOverlayInfo->overlayHeaderList[i].start_mb_x = start_mb_x;
			pOverlayInfo->overlayHeaderList[i].start_mb_y = start_mb_y;
			pOverlayInfo->overlayHeaderList[i].end_mb_x = start_mb_x
										+ (pBitMapInfo->width_aligh16 / 16 - 1);
			pOverlayInfo->overlayHeaderList[i].end_mb_y = start_mb_y
										+ (pBitMapInfo->height_aligh16 / 16 -1);

			pOverlayInfo->overlayHeaderList[i].extra_alpha_flag = 0;
			pOverlayInfo->overlayHeaderList[i].extra_alpha = 8;
			if (i%3 == 0)
				pOverlayInfo->overlayHeaderList[i].overlay_type = LUMA_REVERSE_OVERLAY;
			else if (i%2 == 0 && i!=0)
				pOverlayInfo->overlayHeaderList[i].overlay_type = COVER_OVERLAY;
			else
				pOverlayInfo->overlayHeaderList[i].overlay_type = NORMAL_OVERLAY;


			if (pOverlayInfo->overlayHeaderList[i].overlay_type == COVER_OVERLAY) {
				pOverlayInfo->overlayHeaderList[i].cover_yuv.cover_y = 0xff;
				pOverlayInfo->overlayHeaderList[i].cover_yuv.cover_u = 0xff;
				pOverlayInfo->overlayHeaderList[i].cover_yuv.cover_v = 0xff;
			}

			//pOverlayInfo->overlayHeaderList[i].bforce_reverse_flag = 1;

			pOverlayInfo->overlayHeaderList[i].overlay_blk_addr = pBitMapInfo->argb_addr;
			pOverlayInfo->overlayHeaderList[i].bitmap_size = pBitMapInfo->size;

			logv("blk_%d[%d], start_mb[%d, %d], end_mb[%d, %d], extra_alpha_flag:%d, extra_alpha:%d\n",
								i,
								time_id_list[i],
								pOverlayInfo->overlayHeaderList[i].start_mb_x,
								pOverlayInfo->overlayHeaderList[i].start_mb_y,
								pOverlayInfo->overlayHeaderList[i].end_mb_x,
								pOverlayInfo->overlayHeaderList[i].end_mb_y,
								pOverlayInfo->overlayHeaderList[i].extra_alpha_flag,
								pOverlayInfo->overlayHeaderList[i].extra_alpha);
			logv("overlay_type:%d, cover_yuv[%d, %d, %d], overlay_blk_addr:%p, bitmap_size:%d\n",
								pOverlayInfo->overlayHeaderList[i].overlay_type,
								pOverlayInfo->overlayHeaderList[i].cover_yuv.cover_y,
								pOverlayInfo->overlayHeaderList[i].cover_yuv.cover_u,
								pOverlayInfo->overlayHeaderList[i].cover_yuv.cover_v,
								pOverlayInfo->overlayHeaderList[i].overlay_blk_addr,
								pOverlayInfo->overlayHeaderList[i].bitmap_size);
			//if (i != 5)
			{
				start_mb_x += pBitMapInfo->width_aligh16 / 16;
				start_mb_y += pBitMapInfo->height_aligh16 / 16;
			}
		}

	return;
}*/

int initH264Func(h264_func_t *h264_func, encode_param_t *encode_param)
{
	memset(h264_func, 0, sizeof(h264_func_t));

	//init h264Param
	h264_func->h264Param.bEntropyCodingCABAC = 1;
	h264_func->h264Param.nBitrate = encode_param->bit_rate;
	h264_func->h264Param.nFramerate = encode_param->frame_rate;
	h264_func->h264Param.nCodingMode = VENC_FRAME_CODING;
	h264_func->h264Param.nMaxKeyInterval = encode_param->maxKeyFrame;
	h264_func->h264Param.sProfileLevel.nProfile = VENC_H264ProfileHigh;
	h264_func->h264Param.sProfileLevel.nLevel = VENC_H264Level51;

#ifdef VBR
	h264_func->h264Param.sRcParam.eRcMode = AW_VBR;
#endif
	if (encode_param->qp_min > 0 && encode_param->qp_max) {
		h264_func->h264Param.sQPRange.nMinqp = encode_param->qp_min;
		h264_func->h264Param.sQPRange.nMaxqp = encode_param->qp_max;

	} else {
		h264_func->h264Param.sQPRange.nMinqp = 10;
		h264_func->h264Param.sQPRange.nMaxqp = 50;
	}
	//h264_func->h264Param.bLongRefEnable = 1;
	//h264_func->h264Param.nLongRefPoc = 0;

#if 1
	h264_func->sH264Smart.img_bin_en = 1;
	h264_func->sH264Smart.img_bin_th = 27;
	h264_func->sH264Smart.shift_bits = 2;
	h264_func->sH264Smart.smart_fun_en = 1;
#endif

	//init VencMBModeCtrl
	init_mb_mode(&h264_func->h264MBMode, encode_param);

	//init VencMBInfo
	init_mb_info(&h264_func->MBInfo, encode_param);

	//init VencH264FixQP
	init_fix_qp(&h264_func->fixQP);

	//init VencSuperFrameConfig
	init_super_frame_cfg(&h264_func->sSuperFrameCfg);

	//init VencH264SVCSkip
	init_svc_skip(&h264_func->SVCSkip);

	//init VencH264AspectRatio
	init_aspect_ratio(&h264_func->sAspectRatio);

	//init VencH264AspectRatio
	init_video_signal(&h264_func->sVideoSignal);

	//init CyclicIntraRefresh
	init_intra_refresh(&h264_func->sIntraRefresh);

	//init VencROIConfig
	init_roi(h264_func->sRoiConfig);

	//init proc info
	init_enc_proc_info(&h264_func->sVeProcInfo);

	//init VencOverlayConfig
	//init_overlay_info(&h264_func->sOverlayInfo, encode_param);

	return 0;
}

static int setEncParam(VideoEncoder *pVideoEnc, encode_param_t *encode_param)
{
	int result = 0;
	VeProcSet mProcSet;
	mProcSet.bProcEnable = 1;
	mProcSet.nProcFreq = 30;
	mProcSet.nStatisBitRateTime = 1000;
	mProcSet.nStatisFrRateTime = 1000;
	unsigned int vbv_size = 12*1024*1024;
	VideoEncSetParameter(pVideoEnc, VENC_IndexParamProcSet, &mProcSet);


	if (encode_param->encode_format == VENC_CODEC_H264) {
		result = initH264Func(&encode_param->h264_func, encode_param);
		if (result) {
			loge("initH264Func error, return \n");
			return -1;
		}
		VideoEncSetParameter(pVideoEnc, VENC_IndexParamH264Param, &encode_param->h264_func.h264Param);
		VideoEncSetParameter(pVideoEnc, VENC_IndexParamSetVbvSize, &vbv_size);
		//VideoEncSetParameter(pVideoEnc, VENC_IndexParamH264FixQP, &h264_func.fixQP);
		if (encode_param->test_overlay_flag == 1) {
			VideoEncSetParameter(pVideoEnc, VENC_IndexParamSetOverlay, &encode_param->h264_func.sOverlayInfo);
		}

		VideoEncSetParameter(pVideoEnc, VENC_IndexParamProcSet, &encode_param->h264_func.sVeProcInfo);
#ifdef USE_VIDEO_SIGNAL
		VencH264VideoSignal mVencH264VideoSignal;
		memset(&mVencH264VideoSignal, 0, sizeof(VencH264VideoSignal));
		mVencH264VideoSignal.video_format = DEFAULT;
		mVencH264VideoSignal.full_range_flag = 1;
		mVencH264VideoSignal.src_colour_primaries = VENC_BT709;
		mVencH264VideoSignal.dst_colour_primaries = VENC_BT709;
		VideoEncSetParameter(pVideoEnc, VENC_IndexParamH264VideoSignal, &mVencH264VideoSignal);
#endif
#ifdef DETECT_MOTION
		MotionParam mMotionPara;
		mMotionPara.nMaxNumStaticFrame = 4;
		mMotionPara.nMotionDetectEnable = 1;
		mMotionPara.nMotionDetectRatio = 0;
		mMotionPara.nMV64x64Ratio = 0.01;
		mMotionPara.nMVXTh = 6;
		mMotionPara.nMVYTh = 6;
		mMotionPara.nStaticBitsRatio = 0.2;
		mMotionPara.nStaticDetectRatio = 2;
		VideoEncSetParameter(pVideoEnc, VENC_IndexParamMotionDetectStatus,
												&mMotionPara);
#endif

#ifdef GET_MB_INFO
		VideoEncSetParameter(pVideoEnc, VENC_IndexParamMBInfoOutput, &encode_param->h264_func.MBInfo);
#endif


}


	return 0;
}

static int test_encoder(void)
{
	VencBaseConfig baseConfig;
	VencAllocateBufferParam bufferParam;
	encode_param_t  encode_param;
	VencHeaderData sps_pps_data;
	int result;
	int nAlignW;
	int nAlignH;
	unsigned int testNumber = 0;
	struct test_buf_info *pbuf_in;
	struct test_buf_info *pbuf_out;
	long pts = 0;

	VencInputBuffer inputBuffer;
	VencOutputBuffer outputBuffer;

	pbuf_in = &context->in_buf;
	pbuf_out = &context->out_buf;

	memset(&encode_param, 0, sizeof(encode_param));

	encode_param.src_width = 1280;
	encode_param.src_height = 720;
	encode_param.dst_width = 1280;
	encode_param.dst_height = 720;
	encode_param.bit_rate = 2*1024*1024;
	encode_param.frame_rate = 30;
	encode_param.maxKeyFrame = 30;
	encode_param.encode_format = VENC_CODEC_H264;
	encode_param.encode_frame_num = 50;
	encode_param.test_cycle = 1;

	if (encode_param.dst_width == 3840)
		encode_param.bit_rate = 20*1024*1024;
	else if (encode_param.dst_width == 1920)
		encode_param.bit_rate = 10*1024*1024;
	else if (encode_param.dst_width == 1280)
		encode_param.bit_rate = 6*1024*1024;//6*1024*1024;
	else if (encode_param.dst_width == 640)
		encode_param.bit_rate = 2*1024*1024;
	else if (encode_param.dst_width == 288)
		encode_param.bit_rate = 1*1024*1024;
	else
		encode_param.bit_rate = 4*1024*1024;

	baseConfig.nInputWidth = encode_param.src_width;
	baseConfig.nInputHeight = encode_param.src_height;
	baseConfig.nStride = encode_param.src_width;
	baseConfig.nDstWidth = encode_param.dst_width;
	baseConfig.nDstHeight = encode_param.dst_height;
	baseConfig.bEncH264Nalu = 0;
	baseConfig.eInputFormat = VENC_PIXEL_YUV420P;

	context->venc = VideoEncCreate(VENC_CODEC_H264);
	if (context->venc == NULL) {
		loge("enc create error!");
		return -1;
	}
	result = setEncParam(context->venc, &encode_param);
	if (result) {
		loge("setEncParam error, return");
		return -1;
	}
	VideoEncoderSetFreq(context->venc, 300);
	VideoEncInit(context->venc, &baseConfig);

	VideoEncoderSetDdrMode(context->venc, DDRTYPE_DDR3_32BITS);
	memset(&bufferParam, 0, sizeof(VencAllocateBufferParam));

	// ve require 16-align
	nAlignW = (baseConfig.nInputWidth + 15)& ~15;
	nAlignH = (baseConfig.nInputHeight + 15)& ~15;

	bufferParam.nSizeY = nAlignW*nAlignH;
	bufferParam.nSizeC = nAlignW*nAlignH/2;
	bufferParam.nBufferNum = 1;

	VideoEncGetParameter(context->venc, VENC_IndexParamH264SPSPPS, &sps_pps_data);
	memcpy(pbuf_out->buf+ pbuf_out->write_pos, sps_pps_data.pBuffer, sps_pps_data.nLength);
	pbuf_out->write_pos += sps_pps_data.nLength;
	pbuf_out->data_len += sps_pps_data.nLength;

	AllocInputBuffer(context->venc, &bufferParam);

	while (testNumber < encode_param.encode_frame_num) {
		GetOneAllocInputBuffer(context->venc, &inputBuffer);
		//if (testNumber == 0)
		{
			memcpy(inputBuffer.pAddrVirY, pbuf_in->buf+pbuf_in->read_pos, nAlignW*nAlignH);
			pbuf_in->read_pos += nAlignW*nAlignH;
			logd("ji***read pos:%d w:%d h:%d", pbuf_in->read_pos, nAlignW, nAlignH);
			memcpy(inputBuffer.pAddrVirC, pbuf_in->buf+pbuf_in->read_pos, nAlignW*nAlignH/2);

			VE_LOGD("ji**data:%x %x %x %x %x %x %x %x", *(inputBuffer.pAddrVirC),
				*(inputBuffer.pAddrVirC+1), *(inputBuffer.pAddrVirC+2),
				*(inputBuffer.pAddrVirC+3), *(inputBuffer.pAddrVirC+4),
				*(inputBuffer.pAddrVirC+5), *(inputBuffer.pAddrVirC+6),
				*(inputBuffer.pAddrVirC+7));

			//pbuf_in->read_pos += nAlignW*nAlignH/2;
			pbuf_in->read_pos = 0;
		}

		inputBuffer.bEnableCorp = 0;
		inputBuffer.sCropInfo.nLeft =  240;
		inputBuffer.sCropInfo.nTop	=  240;
		inputBuffer.sCropInfo.nWidth  =  240;
		inputBuffer.sCropInfo.nHeight =  240;
		FlushCacheAllocInputBuffer(context->venc, &inputBuffer);
		pts += 1*1000/encode_param.frame_rate;
		inputBuffer.nPts = pts;
		AddOneInputBuffer(context->venc, &inputBuffer);
		result = VideoEncodeOneFrame(context->venc);
		if (result < 0) {
			logd("encoder error, goto out");
			return -1;
		}
		AlreadyUsedInputBuffer(context->venc, &inputBuffer);
		ReturnOneAllocInputBuffer(context->venc, &inputBuffer);
		result = GetOneBitstreamFrame(context->venc, &outputBuffer);
		if (result == -1) {
			logd("ji******get bit stream fail");
			return -1;
		}

		memcpy(pbuf_out->buf+ pbuf_out->write_pos, outputBuffer.pData0, outputBuffer.nSize0);
		pbuf_out->write_pos += outputBuffer.nSize0;
		pbuf_out->data_len += outputBuffer.nSize0;
		if (outputBuffer.nSize1) {
			memcpy(pbuf_out->buf+ pbuf_out->write_pos, outputBuffer.pData1, outputBuffer.nSize1);
			pbuf_out->write_pos += outputBuffer.nSize1;
			pbuf_out->data_len += outputBuffer.nSize1;
		}

		FreeOneBitStreamFrame(context->venc, &outputBuffer);
		logd("ji**encoder data len:%d", pbuf_out->data_len);
		testNumber++;
		logd("ji**********encoder one frame success");
	}
	return 0;
}

static ssize_t enc_debugfs_input_read(struct file *file, char __user *user_buf,
					  size_t nbytes, loff_t *ppos)
{
	struct test_buf_info *pData  = &context->in_buf;
	int read_len;
	int read_all;
	int end_size;
	loff_t buf_ppos = 0;
	if (pData == NULL || pData->data_len <= 0) {
		VE_LOGD("there is no data currently");
		return 0;
	}

	VE_LOGD("read len:%d usr ptr:%p data len:%d", nbytes, user_buf, pData->data_len);
	if (nbytes > pData->data_len)
		read_all = pData->data_len;
	else
		read_all = nbytes;

	end_size = pData->buf_len - pData->read_pos;
	if (pData->read_pos > pData->write_pos && read_all > end_size) {
		read_len = simple_read_from_buffer(user_buf, end_size, ppos, pData->buf + pData->read_pos,
					   end_size);
		if (read_len < 0) {
			VE_LOGD("read failed!\n");
			return 0;
		}

		pData->data_len -= end_size;
		pData->read_pos = 0;
		end_size = read_all - end_size;
		read_len = simple_read_from_buffer(user_buf, end_size, ppos, pData->buf,
					   end_size);
		if (read_len < 0) {
			VE_LOGD("read failed!\n");
			return 0;
		}
		VE_LOGD("read data len:%d", read_len);
		pData->read_pos += end_size;
		pData->data_len -= end_size;
	} else {
		read_len = simple_read_from_buffer(user_buf, nbytes, &buf_ppos, pData->buf + pData->read_pos,
						   read_all);

		if (read_len < 0) {
			VE_LOGD("read failed!\n");
			return 0;
		}
		VE_LOGD("2read data len:%d read_pos:%d", read_len, pData->read_pos);
		pData->read_pos += read_all;
		pData->data_len -= read_all;
	}
	return read_all;
}

static ssize_t enc_debugfs_input_write(struct file *file, const char __user *user_buf,
					  size_t nbytes, loff_t *ppos)
{
	struct test_buf_info *pData = &context->in_buf;
	int write_len;
	int size_end;
	loff_t buf_ppos = 0;

	if (pData == NULL) {
		VE_LOGD("there is no data currently\n");
		return -1;
	}
	if (nbytes > pData->buf_len - pData->data_len) {
		VE_LOGE("no enough buffer to save data buffer:%d data:%d", pData->buf_len-pData->data_len, nbytes);
		return -ENOSPC;
	}

	size_end = pData->buf_len - pData->write_pos;
	if (pData->write_pos > pData->read_pos && nbytes > size_end) {

		write_len = simple_write_to_buffer(pData->buf + pData->write_pos, size_end,
								ppos, user_buf, size_end);

		if (write_len < 0) {
			VE_LOGD("write buffer error!\n");
			return write_len;
		}
		pData->data_len += size_end;
		pData->write_pos = 0;

		write_len = simple_write_to_buffer(pData->buf + pData->write_pos, nbytes-size_end,
								ppos, user_buf, nbytes-size_end);

		if (write_len < 0) {
			VE_LOGD("write buffer error!\n");
			return write_len;
		}
		pData->data_len += write_len;
		pData->write_pos += write_len;
	} else {
		//VE_LOGD("nbytes:%d ppos:%d", nbytes, *ppos);
		write_len = simple_write_to_buffer(pData->buf + pData->write_pos, nbytes,
								&buf_ppos, user_buf, nbytes);
		/*
		VE_LOGD("data:%x %x %x %x %x %x %x %x", *(pData->buf+pData->write_pos),
				*(pData->buf+pData->write_pos+1), *(pData->buf+pData->write_pos+2),
				*(pData->buf+pData->write_pos+3), *(pData->buf+pData->write_pos+4),
				*(pData->buf+pData->write_pos+5), *(pData->buf+pData->write_pos+6),
				*(pData->buf+pData->write_pos+7));*/

		if (write_len < 0) {
		VE_LOGD("write buffer error!\n");
			return write_len;
		}
		pData->data_len += write_len;
		pData->write_pos += write_len;
	}

	//VE_LOGD("write len:%d pData:%p, usr buf:%p data len:%d", write_len, pData, user_buf, pData->data_len);

	if (pData->data_len == 1280*720*3/2) {
		test_encoder();
	}

	return write_len;
}

static ssize_t enc_debugfs_output_read(struct file *file, char __user *user_buf,
					  size_t nbytes, loff_t *ppos)
{
	struct test_buf_info *pData  = &context->out_buf;
	int read_len;
	int read_all;
	int end_size;
	loff_t buf_ppos = 0;
	if (pData == NULL || pData->data_len <= 0) {
		VE_LOGD("there is no data currently");
		return 0;
	}

	VE_LOGD("read len:%d pData:%p data len:%d", nbytes, pData, pData->data_len);
	if (nbytes > pData->data_len)
		read_all = pData->data_len;
	else
		read_all = nbytes;

	end_size = pData->buf_len - pData->read_pos;
	if (pData->read_pos > pData->write_pos && read_all > end_size) {
		read_len = simple_read_from_buffer(user_buf, end_size, ppos, pData->buf + pData->read_pos,
					   end_size);
		if (read_len < 0) {
			VE_LOGD("read failed!\n");
			return 0;
		}

		pData->data_len -= end_size;
		pData->read_pos = 0;
		end_size = read_all - end_size;
		read_len = simple_read_from_buffer(user_buf, end_size, ppos, pData->buf,
					   end_size);
		if (read_len < 0) {
			VE_LOGD("read failed!\n");
			return 0;
		}
		VE_LOGD("read data len:%d", read_len);
		pData->read_pos += end_size;
		pData->data_len -= end_size;
	} else {
		read_len = simple_read_from_buffer(user_buf, nbytes, &buf_ppos, pData->buf + pData->read_pos,
						   read_all);

		if (read_len < 0) {
			VE_LOGD("read failed!\n");
			return 0;
		}
		VE_LOGD("2read data len:%d", read_len);
		pData->read_pos += read_all;
		pData->data_len -= read_all;
	}
	return read_all;
}

static int enc_debugfs_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;

	return 0;
}

static int enc_debugfs_open(struct inode *inode, struct file *file)
{
	file->private_data = context;
	return 0;
}

static const struct file_operations ve_debugfs_fops2 = {
	.owner = THIS_MODULE,
	.open = enc_debugfs_open,
	.llseek = no_llseek,
	.read = enc_debugfs_output_read,
	.release = enc_debugfs_release,
};

static const struct file_operations ve_debugfs_fops = {
	.owner = THIS_MODULE,
	.open = enc_debugfs_open,
	.llseek = no_llseek,
	.write = enc_debugfs_input_write,
	.read = enc_debugfs_input_read,
	.release = enc_debugfs_release,
};


int enc_debug_register_driver(void)
{
	struct dentry *dent_out;
	struct dentry *dent_in;

	vedir = debugfs_create_dir("enc_test", NULL);

	if (vedir == NULL) {
		VE_LOGE("create ve dir failed!\n");
		return -ENOENT;
	}

	dent_in = debugfs_create_file("input_yuv", 0666, vedir,
				   NULL, &ve_debugfs_fops);
	if (IS_ERR_OR_NULL(dent_in)) {
		VE_LOGE("Unable to create debugfs status file.\n");
		debugfs_remove_recursive(vedir);
		vedir = NULL;
		return -ENODEV;
	}

	dent_out = debugfs_create_file("output_yuv", 0444, vedir,
				   NULL, &ve_debugfs_fops2);
	if (IS_ERR_OR_NULL(dent_out)) {
		VE_LOGE("Unable to create debugfs status file.\n");
		debugfs_remove_recursive(vedir);
		vedir = NULL;
		return -ENODEV;
	}

	context = kmalloc(sizeof(struct test_context), GFP_KERNEL);
	if (context == NULL) {
		VE_LOGE("kmalloc test_context fail\n");
		return -ENOMEM;
	}
	context->in_buf.buf = (char *)vmalloc(TEST_IN_BUF_SIZE);
	if (context->in_buf.buf == NULL) {
		VE_LOGE("kmalloc pData fail\n");
		kfree(context);
		return -ENOMEM;
	}
	context->in_buf.buf_len = TEST_IN_BUF_SIZE;
	context->in_buf.data_len = 0;
	context->in_buf.read_pos = 0;
	context->in_buf.write_pos = 0;

	memset(context->in_buf.buf, 0, TEST_IN_BUF_SIZE);

	context->out_buf.buf = (char *)vmalloc(TEST_OUT_BUF_SIZE);
	if (context->out_buf.buf == NULL) {
		VE_LOGE("kmalloc out buf fail\n");
		kfree(context);
		return -ENOMEM;
	}
	context->out_buf.buf_len = TEST_OUT_BUF_SIZE;
	context->out_buf.data_len = 0;
	context->out_buf.read_pos = 0;
	context->out_buf.write_pos = 0;

	memset(context->out_buf.buf, 0, TEST_OUT_BUF_SIZE);
	return 0;
}

void enc_debug_unregister_driver(void)
{
	if (vedir == NULL)
		return;
	debugfs_remove_recursive(vedir);
	vedir = NULL;
	vfree(context->in_buf.buf);
	context->in_buf.buf = NULL;
	kfree(context);
	context = NULL;
}

static int __init enc_test_init(void)
{
	printk("enc test version 1.0\n");
	return enc_debug_register_driver();
}

static void __exit enc_test_exit(void)
{
	enc_debug_unregister_driver();
}

module_init(enc_test_init);
module_exit(enc_test_exit);


MODULE_AUTHOR("jilinglin");
MODULE_DESCRIPTION("test case for encoder");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0alpha");
MODULE_ALIAS("platform:enc_test");
