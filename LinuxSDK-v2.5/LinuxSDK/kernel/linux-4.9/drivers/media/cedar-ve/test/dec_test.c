/*
 * =====================================================================================
 *
 *  Filename: enc_test.c
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
#include "vdecoder.h"
#include "log.h"

#define ALIGN_XXB(y, x) (((x) + ((y)-1)) & ~((y)-1))
#define TEST_DECODE_NUM 10
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
	VideoDecoder *vdec;

};
#define VE_LOGD(fmt, arg...) printk(KERN_DEBUG"VE: "fmt"\n", ##arg)
#define VE_LOGI(fmt, arg...) printk(KERN_INFO"VE: "fmt"\n", ##arg)
#define VE_LOGW(fmt, arg...) printk(KERN_WARNING"VE: "fmt"\n", ##arg)
#define VE_LOGE(fmt, arg...) printk(KERN_ERR"VE: "fmt"\n", ##arg)

#define TEST_OUT_BUF_SIZE (1280 * 736 * 3/2)
#define TEST_IN_BUF_SIZE (2 * 1024 * 1024)

static struct dentry *vedir;
static struct test_context *context;

static int test_decoder(void)
{
	VConfig VideoConf;
	VideoStreamInfo VideoInfo;
	VideoStreamDataInfo  dataInfo;
	int nRet;
	int testNumber = 0;
	char *pInput = NULL;
	char *pInput2 = NULL;
	char *pOutput = NULL;
	int pInputLen;
	int pInput2Len;
	VideoPicture *pPicture = NULL;
	int PicSize;
	struct test_buf_info *pbuf_in;
	struct test_buf_info *pbuf_out;

	memset(&VideoConf, 0, sizeof(VConfig));
	memset(&VideoInfo, 0, sizeof(VideoStreamInfo));

	pbuf_in = &context->in_buf;
	pbuf_out = &context->out_buf;

	pbuf_out->write_pos = 0;
	pbuf_out->data_len = 0;
	loge("ji********dec start");
	context->vdec = CreateVideoDecoder();
	if (context->vdec == NULL) {
		VE_LOGE("create decoder failed");
		return -1;
	}

	VideoInfo.eCodecFormat = VIDEO_CODEC_FORMAT_MJPEG;
	VideoConf.eOutputPixelFormat  = PIXEL_FORMAT_YV12;
	VideoConf.nDeInterlaceHoldingFrameBufferNum = 0;
	VideoConf.nDisplayHoldingFrameBufferNum = 1;
	VideoConf.nRotateHoldingFrameBufferNum = 0;
	VideoConf.nDecodeSmoothFrameBufferNum = 0;
	VideoConf.nAlignStride = 16;

	logd("nCodecFormat = 0x%x", VideoInfo.eCodecFormat);
	nRet = InitializeVideoDecoder(context->vdec, &VideoInfo, &VideoConf);
	if (nRet != 0) {
		loge("decoder demom initialize video decoder fail.");
		DestroyVideoDecoder(context->vdec);
		context->vdec = NULL;
		return -1;
	}

	while (testNumber < TEST_DECODE_NUM) {
		nRet = RequestVideoStreamBuffer(context->vdec, pbuf_in->data_len,
										&pInput, &pInputLen, &pInput2, &pInput2Len, 0);
		if (nRet != 0) {
			VE_LOGE(" RequestVideoStreamBuffer fail. request size: %d", pbuf_in->data_len);
		}
		//if (testNumber == 0)
		{
			memcpy(pInput, pbuf_in->buf+pbuf_in->read_pos, pInputLen);
			pbuf_in->read_pos += pInputLen;
			loge("ji***read pos:%d data len:%d", pbuf_in->read_pos, pInputLen);
			if (pInput2 != NULL)
				memcpy(pInput2, pbuf_in->buf+pbuf_in->read_pos, pInput2Len);

			loge("ji**data:%x %x %x %x %x %x %x %x", *(pInput), *(pInput+1), *(pInput+2),
				*(pInput+3), *(pInput+4), *(pInput+5), *(pInput+6), *(pInput+7));

			//pbuf_in->read_pos += nAlignW*nAlignH/2;
			pbuf_in->read_pos = 0;
		}

		memset(&dataInfo, 0, sizeof(VideoStreamDataInfo));
		dataInfo.pData			= pInput;
		dataInfo.nLength	  = pInputLen;
		dataInfo.nPts		   = 0;
		dataInfo.nPcr		   = 0;
		dataInfo.bIsFirstPart = 1;
		dataInfo.bIsLastPart = 1;
		nRet = SubmitVideoStreamData(context->vdec, &dataInfo, 0);
		if (nRet < 0) {
			logd("submit error, goto out");
			return -1;
		}
		nRet = DecodeVideoStream(context->vdec, 0, 0, 0, 0);
		if (nRet < 0) {
			loge("encoder error, goto out");
			return -1;
		}
		if (nRet == VDECODE_RESULT_KEYFRAME_DECODED ||
			nRet == VDECODE_RESULT_FRAME_DECODED) {
			VE_LOGE("decoder success");
		} else {
			loge("decoder ret:%d", nRet);
		}
		if (ValidPictureNum(context->vdec, 0)) {
			pPicture = RequestPicture(context->vdec, 0/* the major stream*/);
			if (pPicture != NULL) {
				loge("ji****width:%d height:%d p0:%p", pPicture->nWidth, pPicture->nHeight, pPicture->pData0);
				pOutput = pPicture->pData0;
				loge("ji**data:%x %x %x %x %x %x %x %x", *(pOutput), *(pOutput+1), *(pOutput+2),
				*(pOutput+3), *(pOutput+4), *(pOutput+5), *(pOutput+6), *(pOutput+7));
				PicSize = pPicture->nWidth *pPicture->nHeight * 3 / 2;
				memcpy(pbuf_out->buf + pbuf_out->write_pos, pPicture->pData0, PicSize);
				pbuf_out->write_pos += PicSize;
				pbuf_out->data_len += PicSize;

				ReturnPicture(context->vdec, pPicture);
				if (testNumber < TEST_DECODE_NUM -1) {
					//for last decoder time
					pbuf_out->write_pos = 0;
					pbuf_out->data_len = 0;
				}
			}
		}
		loge("ji**decoder data len:%d", pbuf_out->data_len);
		testNumber++;
	}
	DestroyVideoDecoder(context->vdec);
	return 0;
}

static ssize_t dec_debugfs_input_read(struct file *file, char __user *user_buf,
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

static ssize_t dec_debugfs_input_write(struct file *file, const char __user *user_buf,
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
		VE_LOGD("nbytes:%d ppos:%d", nbytes, *ppos);
		write_len = simple_write_to_buffer(pData->buf + pData->write_pos, nbytes,
								&buf_ppos, user_buf, nbytes);

		VE_LOGD("data:%x %x %x %x %x %x %x %x", *(pData->buf+pData->write_pos),
				*(pData->buf+pData->write_pos+1), *(pData->buf+pData->write_pos+2),
				*(pData->buf+pData->write_pos+3), *(pData->buf+pData->write_pos+4),
				*(pData->buf+pData->write_pos+5), *(pData->buf+pData->write_pos+6),
				*(pData->buf+pData->write_pos+7));

		if (write_len < 0) {
			VE_LOGD("write buffer error!\n");
			return write_len;
		}
		pData->data_len += write_len;
		pData->write_pos += write_len;
	}

	VE_LOGD("write len:%d pData:%p, usr buf:%p data len:%d", write_len, pData, user_buf, pData->data_len);

	if (write_len < 4096) {
		test_decoder();
		VE_LOGE("test ok, we set write pos to zero");
		pData->data_len = 0;
		pData->write_pos = 0;
	}

	return write_len;
}

static ssize_t dec_debugfs_output_read(struct file *file, char __user *user_buf,
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

	VE_LOGE("read len:%d pData:%p data len:%d", nbytes, pData, pData->data_len);
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
		VE_LOGE("read data len:%d", read_len);
		pData->read_pos += end_size;
		pData->data_len -= end_size;
	} else {
		read_len = simple_read_from_buffer(user_buf, nbytes, &buf_ppos, pData->buf + pData->read_pos,
						   read_all);

		if (read_len < 0) {
			VE_LOGD("read failed!\n");
			return 0;
		}
		VE_LOGE("2read data len:%d", read_len);
		pData->read_pos += read_all;
		pData->data_len -= read_all;
	}
	return read_all;
}

static int dec_debugfs_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;

	return 0;
}

static int dec_debugfs_open(struct inode *inode, struct file *file)
{
	file->private_data = context;
	return 0;
}

static const struct file_operations ve_debugfs_fops2 = {
	.owner = THIS_MODULE,
	.open = dec_debugfs_open,
	.llseek = no_llseek,
	.read = dec_debugfs_output_read,
	.release = dec_debugfs_release,
};

static const struct file_operations ve_debugfs_fops = {
	.owner = THIS_MODULE,
	.open = dec_debugfs_open,
	.llseek = no_llseek,
	.write = dec_debugfs_input_write,
	.read = dec_debugfs_input_read,
	.release = dec_debugfs_release,
};


int dec_debug_register_driver(void)
{
	struct dentry *dent_out;
	struct dentry *dent_in;

	vedir = debugfs_create_dir("dec_test", NULL);

	if (vedir == NULL) {
		VE_LOGE("create ve dir failed!\n");
		return -ENOENT;
	}

	dent_in = debugfs_create_file("input_stream", 0666, vedir,
				   NULL, &ve_debugfs_fops);
	if (IS_ERR_OR_NULL(dent_in)) {
		VE_LOGE("Unable to create debugfs status file.\n");
		debugfs_remove_recursive(vedir);
		vedir = NULL;
		return -ENODEV;
	}

	dent_out = debugfs_create_file("output_stream", 0444, vedir,
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

void dec_debug_unregister_driver(void)
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

static int __init dec_test_init(void)
{
	printk("dec test version 1.0\n");
	return dec_debug_register_driver();
}

static void __exit dec_test_exit(void)
{
	dec_debug_unregister_driver();
}

module_init(dec_test_init);
module_exit(dec_test_exit);


MODULE_AUTHOR("jilinglin");
MODULE_DESCRIPTION("test case for decoder");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0alpha");
MODULE_ALIAS("platform:dec_test");
