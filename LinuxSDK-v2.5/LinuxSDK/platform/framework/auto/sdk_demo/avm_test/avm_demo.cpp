/*
 * drivers/media/platform/sunxi-vin/vin_test/mplane_image/csi_test_mplane.c
 *
 * Copyright (c) 2014 softwinner.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/*
 * zw
 * for csi & isp test
 */
#include "avm_demo.h"

#define CLEAR(x) (memset(&(x), 0, sizeof(x)))
#define ALIGN_4K(x) (((x) + (4095)) & ~(4095))
#define ALIGN_16B(x) (((x) + (15)) & ~(15))

#define video_s_ctrl 0

#define DMAFD_TEST 1

#ifdef DMAFD_TEST
int egl_param[4];
extern "C" int create_textture(void *param);
extern "C" int info_aframe(int *dmafd);
#endif

#define DISP_ION_NUM 8
typedef struct _disp_ion_
{
	void * vir;
	unsigned int len;
	int dmafd;
}disp_ion;
disp_ion gDispTab[DISP_ION_NUM];
disp_ion gDispTab0[DISP_ION_NUM];
disp_ion gDispTab1[DISP_ION_NUM];
disp_ion gDispTab2[DISP_ION_NUM];
disp_ion gDispTab3[DISP_ION_NUM];

enum v4l2_memory memtype = V4L2_MEMORY_USERPTR;//=V4L2_MEMORY_MMAP V4L2_MEMORY_USERPTR V4L2_MEMORY_DMABUF 


struct size {
	int width;
	int height;
};
struct buffer {
	void *start[3];
	int length[3];
};

typedef enum {
	TVD_PL_YUV420 = 0,
	TVD_MB_YUV420 = 1,
	TVD_PL_YUV422 = 2,
} TVD_FMT_T;

struct disp_screen {
	int x;
	int y;
	int w;
	int h;
};

struct test_layer_info {
	int screen_id;
	int layer_id;
	int mem_id;
	disp_layer_config layer_config;
	int addr_map;
	int width, height;/* screen size */
	int dispfh;/* device node handle */
	int fh;/* picture resource file handle */
	int mem;
	int clear;/* is clear layer */
	char filename[32];
	int full_screen;
	unsigned int pixformat;
	disp_output_type output_type;
};

/**
 * tvd_dev info
 */
struct tvd_dev {
	unsigned int ch_id;
	unsigned int height;
	unsigned int width;
	unsigned int interface;
	unsigned int system;
	unsigned int row;
	unsigned int column;
	unsigned int ch0_en;
	unsigned int ch1_en;
	unsigned int ch2_en;
	unsigned int ch3_en;
	unsigned int pixformat;
	struct test_layer_info layer_info;
	int frame_no_to_grap;
	FILE *raw_fp;
};
struct tvd_dev dev;

static char path_name[20];
static char dev_name0[20];
static char dev_name1[20];
static char dev_name2[20];
static char dev_name3[20];
static char *dev_addr[4];
static int fd[4] = {0};
static int isp0_fd = -1;
static int isp1_fd = -1;

struct buffer *buffers;
struct buffer *buffers0;
struct buffer *buffers1;
struct buffer *buffers2;
struct buffer *buffers3;
static unsigned int n_buffers[4] = {0};

struct size input_size;

unsigned int req_frame_num = 8;
unsigned int count;
unsigned int nplanes;
int dev_id[4];
unsigned int fps = 30;
unsigned int wdr_mode;

#define ROT_90 0

static int disp_disable(void)
{
#if display_frame
	int ret;
	unsigned long arg[6];
	struct disp_layer_config disp;

	/* release memory && clear layer */
	arg[0] = 0;
	arg[1] = 0;
	arg[2] = 0;
	arg[3] = 0;
	ioctl(dev.layer_info.dispfh, DISP_LAYER_DISABLE, (void *)arg);

	/*close channel 0*/
	memset(&disp, 0, sizeof(disp_layer_config));
	disp.channel = 0;
	disp.layer_id = 0;
	disp.enable = 0;
	arg[0] = dev.layer_info.screen_id;
	arg[1] = (unsigned long)&disp;
	arg[2] = 1;
	arg[3] = 0;
	ret = ioctl(dev.layer_info.dispfh, DISP_LAYER_SET_CONFIG, (void *)arg);
	if (ret != 0)
		printf("disp_disable:disp_set_addr fail to set layer info\n");

	/*close channel 2*/
	memset(&disp, 0, sizeof(disp_layer_config));
	disp.channel = 2;
	disp.layer_id = 0;
	disp.enable = 0;
	arg[0] = dev.layer_info.screen_id;
	arg[1] = (unsigned long)&disp;
	arg[2] = 1;
	arg[3] = 0;
	ret = ioctl(dev.layer_info.dispfh, DISP_LAYER_SET_CONFIG, (void *)arg);
	if (ret != 0)
		printf("disp_disable:disp_set_addr fail to set layer info\n");

	return ret;
#else
	return 0;
#endif
}


#define EGLIMG_DMAFD_MAX        32
int setup_dmafd2eglimg(EGLDisplay egl_display, int dma_fd, EGLImageKHR *eglImage)
{
	int atti = 0;
	EGLint attribs[30];
	int ret;
	int i =0;
	EGLImageKHR img;

	if (dma_fd < 0)
	{
		printf("dmafd parameter err.\n");
		return (int)NULL;
	}

	atti = 0;
	attribs[atti++] = EGL_WIDTH;
	attribs[atti++] = CAMERA_WIDTH;
	attribs[atti++] = EGL_HEIGHT;
	attribs[atti++] = CAMERA_HEIGHT;
	attribs[atti++] = EGL_LINUX_DRM_FOURCC_EXT;
	attribs[atti++] = DRM_FORMAT_NV21;//DRM_FORMAT_NV21;
	attribs[atti++] = EGL_DMA_BUF_PLANE0_FD_EXT;
	attribs[atti++] = dma_fd;
	attribs[atti++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
	attribs[atti++] = 0;
	attribs[atti++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
	attribs[atti++] = CAMERA_WIDTH;

	attribs[atti++] = EGL_DMA_BUF_PLANE1_FD_EXT;
	attribs[atti++] = dma_fd;
	attribs[atti++] = EGL_DMA_BUF_PLANE1_OFFSET_EXT;
	attribs[atti++] =CAMERA_WIDTH * CAMERA_HEIGHT;
	attribs[atti++] = EGL_DMA_BUF_PLANE1_PITCH_EXT;
	attribs[atti++] = CAMERA_WIDTH;

	attribs[atti++] = EGL_YUV_COLOR_SPACE_HINT_EXT;
	attribs[atti++] = EGL_ITU_REC709_EXT;
	attribs[atti++] = EGL_SAMPLE_RANGE_HINT_EXT;
	attribs[atti++] = EGL_YUV_FULL_RANGE_EXT;
	attribs[atti++] = EGL_NONE;

	img = eglCreateImageKHR(egl_display, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, 0, attribs);
#if 1
//	printf("===========dma_fd=%d , dmafd_w=%d ,  dmafd_h=%d \n",dma_fd, dmafd_w,  dmafd_h);
	if (img == EGL_NO_IMAGE_KHR)
	{
		printf("Error %s(): failed: 0x%08X, dmafd=0x%x \n", __func__, glGetError(), dma_fd);
		return (int)NULL;
	}
#endif
	//usleep(200000);

	*eglImage = img;
	
	return 0;
}

static int search_dmafd(int dmafd, EGLImageKHR *p_img, int *p_index)
{
	int ret=-1;
	EGLImageKHR img;
	int i;

	for (i = 0; i < EGLIMG_DMAFD_MAX; i++)
	{
		if (g_eglimg_dmafd_array[i].dmafd == dmafd)
		{
			img = g_eglimg_dmafd_array[i].eglimg;
			ret = 0;
			break;
		}
		if (g_eglimg_dmafd_array[i].dmafd == -1)
		{
			break;
		}
	}

	if (p_img == NULL)
	{
		printf("p img NULL exit -1\n");
	}
	else
	{
		*p_img = img;
	}

	if (p_index == NULL)
	{
		printf("p index NULL exit -1\n");
	}
	else
	{
		*p_index = i;
	}
	return ret;
}

GLeglImageOES bind_dmafd(int dmafd, EGLDisplay dpy)
{
	int ret;
	EGLImageKHR img;
	int index  ;

	if(search_dmafd(dmafd, &img, &index) != 0)
	{
		ret = setup_dmafd2eglimg(dpy, dmafd, &img);
		if(ret != 0)
		{
			printf("setup_dmafd2eglimg err\n");
		}
		else
		{
			g_eglimg_dmafd_array[index].dmafd = dmafd;
			g_eglimg_dmafd_array[index].eglimg = img;
		}
	}
	return img;
}

int init_eglimg_dmafd(void)
{
	unsigned long i;
	for (i = 0; i < EGLIMG_DMAFD_MAX; i++)
	{
		g_eglimg_dmafd_array[i].dmafd = -1;
		g_eglimg_dmafd_array[i].eglimg = (void *)-1;
	}
	return 0;
}
//=================================================================

void load_2d_3d_bowModle()
{

xcb_avmlib_stitching_Init(&avm_stitching_Handle, "./avm_license");//

xcb_avmlib_stitching_Gen_VBO(NULL, "./gpu_data_default/BowlModel_2D",
                               &avm_stitching_Handle, 0);

xcb_avmlib_stitching_Gen_VBO(NULL, "./gpu_data_default/BowlModel",
                               &avm_stitching_Handle, 1);
}

void Render_2D()
{
  fGreyRatio[0] = 1;
  fGreyRatio[1] = 1;
  fGreyRatio[2] = 1;
  fGreyRatio[3] = 1;
#if __stdlib__
  camera_state.camera_state[0] = true;
  camera_state.camera_state[1] = true;
  camera_state.camera_state[2] = true;
  camera_state.camera_state[3] = true;
  xcb_avmlib_stitching_TopView_Render(&avm_stitching_Handle, cameraParameter, &camera_state, texture_id, 0, &fGreyRatio[0]);
#else

  xcb_avmlib_stitching_TopView_Render(&avm_stitching_Handle, cameraParameter, texture_id, 0, &fGreyRatio[0]);
#endif

//  xcb_avmlib_car_Render_bottom(&avm_car_Handle, cameraParameter, 0);
//  xcb_avmlib_car_Render_2d(&avm_car_Handle, cameraParameter);
  //xcb_avm_line_dynamic_bird_Render(&avmLineDynamicBirdHandle);//画俯视图轨迹线
}
void Render_3D()
{
  fGreyRatio[0] = 1;
  fGreyRatio[1] = 1;
  fGreyRatio[2] = 1;
  fGreyRatio[3] = 1;
#if __stdlib__
  camera_state.camera_state[0] = true;
  camera_state.camera_state[1] = true;
  camera_state.camera_state[2] = true;
  camera_state.camera_state[3] = true;
  xcb_avmlib_stitching_TopView_Render(&avm_stitching_Handle, cameraParameter3D,
                                      &camera_state, texture_id, 1, &fGreyRatio[0]);
#else
  xcb_avmlib_stitching_TopView_Render(&avm_stitching_Handle, cameraParameter3D,
                                      texture_id, 1, &fGreyRatio[0]);
#endif

//  xcb_avmlib_car_Render_bottom(&avm_car_Handle, cameraParameter3D, 1);

//  xcb_avmlib_car_Render_3d(&avm_car_Handle, cameraParameter3D, 0.2);
}

void set_single_2D_3D_vision_viewParam()//相机虚拟视角
{
  cameraParameter.win_x = 10;
  cameraParameter.win_y = 0;
  cameraParameter.win_width = 400;
  cameraParameter.win_height = 800;
  cameraParameter.fov = 80;
  cameraParameter.aspect_ratio = 4.0 / 4.8; //5.4
  cameraParameter.ex = 0;
  cameraParameter.ey = 0;
  cameraParameter.ez = 6000;
  cameraParameter.tx = 0;
  cameraParameter.ty = 0;
  cameraParameter.tz = -3000;
#if __stdlib__
  xcb_avmlib_2d_Set_ViewParameter(cameraParameter, 90);
#else
  xcb_avmlib_2d_Set_ViewParameter(cameraParameter);
#endif
  cameraParameter3D.win_x = 425;
  cameraParameter3D.win_y = 0;
  cameraParameter3D.win_width = 840;
  cameraParameter3D.win_height = 800;
  cameraParameter3D.fov = 35.0;
  cameraParameter3D.aspect_ratio = 84 / 48.0;
  cameraParameter3D.ex = 0;
  cameraParameter3D.ey = 0;
  cameraParameter3D.ez = 3000;
  cameraParameter3D.tx = 0;
  cameraParameter3D.ty = 0;
  cameraParameter3D.tz = 150;

#if __stdlib__
  xcb_avmlib_3d_Set_ViewParameter(cameraParameter3D, 0, 0, 3000);
#else
  xcb_avmlib_3d_Set_ViewParameter(cameraParameter3D);
#endif
  cameraSingle.win_x = 425;
  cameraSingle.win_y = 0;
  cameraSingle.win_width = 840;
  cameraSingle.win_height = 800;
  cameraSingle.fov = 80.0;
  cameraSingle.aspect_ratio = 84 / 48.0;
  cameraSingle.ex = 0;
  cameraSingle.ey = 0;
  cameraSingle.ez = 800;
  cameraSingle.tx = 0;
  cameraSingle.ty = 0;
  cameraSingle.tz = 0;

#if __stdlib__
  xcb_avmlib_2d_Set_ViewParameter(cameraSingle, 90);
#else
  xcb_avmlib_2d_Set_ViewParameter(cameraSingle);
#endif
}


void open3d_rotate_trail_data()
{
    FILE* fp;

  fp = fopen("./gpu_data_default/tt", "rb");
  if (!fp)
  {
    printf("the file tt is not exit!\n");
  }
  fread(&ntt, sizeof(int), 1, fp);
  printf("the path PTS NUM:%d", ntt);
  p = (double *)malloc(sizeof(double) * ntt * 2);
  fread(p, sizeof(double), ntt * 2, fp);
  fclose(fp);
}

void load_single_view_model()
{
#ifdef ISDMA
  avm_single_Handle.isDMA = true;
#endif
  int addnum = 200;

  xcb_avmlib_single_Init(&avm_single_Handle); // 单视图初始化
  xcb_avmlib_single_Gen_VBO(NULL, "./gpu_data_default/SingleModel",
                            &avm_single_Handle); //"./auth_data/SingleModel"    // 读取单视图 vbo 模型文件

  xcb_avmlib_sideView_Gen_VBO(NULL, "./gpu_data_default/SideViewModelF", &avm_single_Handle, 0); // 读取侧面视图 vbo 模型文件
  xcb_avmlib_sideView_Gen_VBO(NULL, "./gpu_data_default/SideViewModelB", &avm_single_Handle, 1);
  xcb_avmlib_sideView_Gen_VBO(NULL, "./gpu_data_default/SideViewModelU", &avm_single_Handle, 2);
  xcb_avmlib_wideAngleView_Gen_VBO(NULL, "./gpu_data_default/BroadViewModel", &avm_single_Handle);
  
}

//+++++++++++++++++++++++++++++++++++ RECODER_TEST +++++++++++++++++++++++++++++++++//
#if RECODER_TEST
void Rec_usernotifyCallback(int32_t msgType, int32_t ext1, int32_t ext2, void *user)
{
	LOGE("msgType =%d-----data=%p-----%d)", msgType, user);

	if ((msgType & CAMERA_MSG_ERROR) == CAMERA_MSG_ERROR) {
		LOGE("(msgType =CAMERA_MSG_ERROR)");

	}
	#if 0
	if ((msgType & CAMERA_MSG_DVR_NEW_FILE) == CAMERA_MSG_DVR_NEW_FILE) {
		dvr_factory *p_dvr = (dvr_factory *) user;
		LOGE("(msgType =CAMERA_MSG_DVR_NEW_FILE camera %d idx =%d)", p_dvr->mCameraId, ext1);
	}

	if ((msgType & CAMERA_MSG_DVR_STORE_ERR) == CAMERA_MSG_DVR_STORE_ERR) {
		LOGE("msgType =CAMERA_MSG_DVR_STORE_ERR)");
		dvr_factory *p_dvr = (dvr_factory *) user;
		p_dvr->mRecordCamera->storage_state = 0;
		p_dvr->stopRecord();
	}
	#endif
}

void Rec_userdataCallback(int32_t msgType, char *dataPtr, camera_frame_metadata_t * metadata,void *user)
{
    
}

void Rec_userdataCallbackTimestamp(nsecs_t timestamp, int32_t msgType, char *dataPtr, void *user)
{
	V4L2BUF_t *pbuf = (V4L2BUF_t *) (dataPtr );
}

#endif
//+++++++++++++++++++++++++++++++++++ RECODER_TEST +++++++++++++++++++++++++++++++++//

#if 1
void *AVM_Camera(void *arg)//全景线程入口
#else
void AVM_Camera(void)
#endif
{
	system("fbinit");
	xcb_avmlib_Window_Init(1280, 800, egl_display, egl_surface);//显示窗口初始化 屏幕大小1280*480
#ifdef ISDMA
	xcb_avmlib_Gen_TextureBuffer_DMA(8, texture_id);
	avm_stitching_Handle.isDMA = true;
	printf("Thread DMA!\r\n");
#else
	xcb_avmlib_Gen_TextureBuffer(8, texture_id);
	printf("Thread not DMA!\r\n");
#endif
	load_2d_3d_bowModle();//加载2D3D 拼接模型
	load_single_view_model();
	set_single_2D_3D_vision_viewParam();//设置2D 3D虚拟相机位置

	open3d_rotate_trail_data();//读取3D旋转轨迹

	while(1)
	{
	  pthread_cond_wait(&cond[0], &mutex[0]);
	  pthread_cond_wait(&cond[1], &mutex[1]);
	  pthread_cond_wait(&cond[2], &mutex[2]);
	  pthread_cond_wait(&cond[3], &mutex[3]);
      for (int i = 0; i < 4; i++)
      {
        xcb_avmlib_Update_TextureBuffer_DMA(elg_Image[i], texture_id, i);
      }	

		xcb_avmlib_Clean_Screen(0.1, 0.1, 0.1, 0.5);//清屏
    
		camera_state.camera_state[0] = true;
		camera_state.camera_state[1] = true;
		camera_state.camera_state[2] = true;
		camera_state.camera_state[3] = true;
		Render_2D();//渲染2D拼接画面
		{

			if (NTT >= ntt)
			{
			NTT = 0;
			}
			cameraParameter3D.ex = p[NTT * 2];
			cameraParameter3D.ey = p[NTT * 2 + 1];
			NTT++;
			#if __stdlib__
		//	xcb_avmlib_3d_Set_ViewParameter(cameraParameter3D, cameraParameter3D.ex, cameraParameter3D.ey, cameraParameter3D.ez);//可以根据p里面的位置进行旋转3D画面（tt文件保存的是运动轨迹）
			xcb_avmlib_3d_Set_ViewParameter(cameraParameter3D, 0, 0,3000);
			#else
			xcb_avmlib_3d_Set_ViewParameter(cameraParameter3D);
			#endif
			Render_3D();//3D拼接渲染
		}
		xcb_avmlib_swapbuffer(egl_display, egl_surface);//渲染到屏幕
	}
#if 1
	pthread_exit(NULL);
#endif
}

static struct timeval gRunTimeEnd;
static long long gtime1 = 0;
static long long gtime2 = 0;
static long long gtime3 = 0;


static void terminate(int sig_no)
{
	printf("Got signal %d, exiting ...\n", sig_no);
	disp_disable();
	usleep(20*1000);
	exit(1);
}

static void install_sig_handler(void)
{
	signal(SIGBUS, terminate);
	signal(SIGFPE, terminate);
	signal(SIGHUP, terminate);
	signal(SIGILL, terminate);
	signal(SIGKILL, terminate);
	signal(SIGINT, terminate);
	signal(SIGIOT, terminate);
	signal(SIGPIPE, terminate);
	signal(SIGQUIT, terminate);
	signal(SIGSEGV, terminate);
	signal(SIGSYS, terminate);
	signal(SIGTERM, terminate);
	signal(SIGTRAP, terminate);
	signal(SIGUSR1, terminate);
	signal(SIGUSR2, terminate);
}

static int req_frame_buffers0(int device_id)
{
	unsigned int i;
	struct v4l2_requestbuffers req;
	struct v4l2_exportbuffer exp;

	CLEAR(req);
	req.count = req_frame_num;	// 8
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	req.memory = memtype;//V4L2_MEMORY_USERPTR;//V4L2_MEMORY_MMAP;
	if (-1 == ioctl(fd[device_id], VIDIOC_REQBUFS, &req)) {
		printf("VIDIOC_REQBUFS error\n");
		pthread_mutex_unlock(&mutex[device_id]);
		return -1;
	}

	buffers0 = (buffer *)calloc(req.count, sizeof(*buffers0));

	for (n_buffers[device_id] = 0; n_buffers[device_id] < req.count; ++n_buffers[device_id]) {
		struct v4l2_buffer buf;

		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		buf.memory = memtype;//V4L2_MEMORY_USERPTR;//V4L2_MEMORY_MMAP;
		buf.index = n_buffers[device_id];
		buf.length = nplanes;
		buf.m.planes =
		    (struct v4l2_plane *)calloc(nplanes,
						sizeof(struct v4l2_plane));
		if (buf.m.planes == NULL) {
			printf("buf.m.planes calloc failed!\n");
			pthread_mutex_unlock(&mutex[device_id]);
			return -1;
		}
		if (-1 == ioctl(fd[device_id], VIDIOC_QUERYBUF, &buf)) {
			printf("VIDIOC_QUERYBUF error\n");
			free(buf.m.planes);
			pthread_mutex_unlock(&mutex[device_id]);
			return -1;
		}
		printf("VIDIOC_QUERYBUF buf.m.planes[j].length=%d\n",buf.m.planes[0].length);
		switch (buf.memory) {
		case V4L2_MEMORY_MMAP:
			for (i = 0; i < nplanes; i++) {
				buffers0[n_buffers[device_id]].length[i] = buf.m.planes[i].length;
				buffers0[n_buffers[device_id]].start[i] =
					mmap(NULL,/* start anywhere */
					 buf.m.planes[i].length,
					 PROT_READ | PROT_WRITE,/* required */
					 MAP_SHARED, /* recommended */
					 fd[device_id], buf.m.planes[i].m.mem_offset);

				if (buffers0[n_buffers[device_id]].start[i] == MAP_FAILED) {
					printf("mmap failed\n");
					free(buf.m.planes);
					return -1;
				}
			}
			free(buf.m.planes);
			break;
		case V4L2_MEMORY_USERPTR:
		case V4L2_MEMORY_DMABUF:
			for (int j = 0; j < nplanes; j++) {
				buffers0[n_buffers[device_id]].length[j] = ALIGN_16B(ALIGN_16B(input_size.width)*ALIGN_16B(input_size.height)*3/2);
				buffers0[n_buffers[device_id]].start[j]= (void *)IonAlloc(buffers0[n_buffers[device_id]].length[j]);
				if (buffers0[n_buffers[device_id]].start[j] == NULL){
					 printf("Camera v4l2QueryBuf buffer allocate ERROR");
					 pthread_mutex_unlock(&mutex[device_id]);
					 return -1;
				}
				buf.m.planes[j].m.userptr = (unsigned long)buffers0[n_buffers[device_id]].start[j];
				buf.m.planes[j].length = buffers0[n_buffers[device_id]].length[j];
			//printf("buffers[%d].start[%d]=%lx\nbuf.m.planes[%d].m.userptr=%lx\n",n_buffers,j,buffers[n_buffers].start[j],j,buf.m.planes[j].m.userptr);
			//printf("Camera[%d] v4l2QueryBuf mem[%d]:0x%lx,len:%d,fd:%d,w*h:%d,%d", mCameraId,i, (unsigned long)buffers[n_buffers].start[i], mMapMem.length, mMapMem.nShareBufFd[i],mFrameWidth,mFrameHeight);
			}
			break;
		}
	}

	for (i = 0; i < n_buffers[device_id]; ++i) {
		struct v4l2_buffer buf;

		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		buf.memory = memtype;//V4L2_MEMORY_USERPTR;//V4L2_MEMORY_MMAP;
		buf.index = i;
		buf.length = nplanes;
		buf.m.planes =
		    (struct v4l2_plane *)calloc(nplanes,
						sizeof(struct v4l2_plane));
			for (int j = 0; j < nplanes; j++) {
				buf.m.planes[j].m.userptr = (unsigned long)buffers0[i].start[j];
				buf.m.planes[j].length = buffers0[i].length[j];
			}


		if (-1 == ioctl(fd[device_id], VIDIOC_QBUF, &buf)) {
			printf("VIDIOC_QBUF failed\n");
			free(buf.m.planes);
			pthread_mutex_unlock(&mutex[device_id]);
			return -1;
		}
		free(buf.m.planes);
	}
	return 0;
}

static int req_frame_buffers1(int device_id)
{
	unsigned int i;
	struct v4l2_requestbuffers req;
	struct v4l2_exportbuffer exp;

	CLEAR(req);
	req.count = req_frame_num;	// 8
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	req.memory = memtype;//V4L2_MEMORY_USERPTR;//V4L2_MEMORY_MMAP;
	if (-1 == ioctl(fd[device_id], VIDIOC_REQBUFS, &req)) {
		printf("VIDIOC_REQBUFS error\n");
		pthread_mutex_unlock(&mutex[device_id]);
		return -1;
	}

	buffers1 = (buffer *)calloc(req.count, sizeof(*buffers1));

	for (n_buffers[device_id] = 0; n_buffers[device_id] < req.count; ++n_buffers[device_id]) {
		struct v4l2_buffer buf;

		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		buf.memory = memtype;//V4L2_MEMORY_USERPTR;//V4L2_MEMORY_MMAP;
		buf.index = n_buffers[device_id];
		buf.length = nplanes;
		buf.m.planes =
		    (struct v4l2_plane *)calloc(nplanes,
						sizeof(struct v4l2_plane));
		if (buf.m.planes == NULL) {
			printf("buf.m.planes calloc failed!\n");
			pthread_mutex_unlock(&mutex[device_id]);
			return -1;
		}
		if (-1 == ioctl(fd[device_id], VIDIOC_QUERYBUF, &buf)) {
			printf("VIDIOC_QUERYBUF error\n");
			free(buf.m.planes);
			pthread_mutex_unlock(&mutex[device_id]);
			return -1;
		}
		printf("VIDIOC_QUERYBUF buf.m.planes[j].length=%d\n",buf.m.planes[0].length);
		switch (buf.memory) {
		case V4L2_MEMORY_MMAP:
			for (i = 0; i < nplanes; i++) {
				buffers1[n_buffers[device_id]].length[i] = buf.m.planes[i].length;
				buffers1[n_buffers[device_id]].start[i] =
					mmap(NULL,/* start anywhere */
					 buf.m.planes[i].length,
					 PROT_READ | PROT_WRITE,/* required */
					 MAP_SHARED, /* recommended */
					 fd[device_id], buf.m.planes[i].m.mem_offset);

				if (buffers1[n_buffers[device_id]].start[i] == MAP_FAILED) {
					printf("mmap failed\n");
					free(buf.m.planes);
					return -1;
				}
			}
			free(buf.m.planes);
			break;
		case V4L2_MEMORY_USERPTR:
		case V4L2_MEMORY_DMABUF:
			for (int j = 0; j < nplanes; j++) {
				buffers1[n_buffers[device_id]].length[j] = ALIGN_16B(ALIGN_16B(input_size.width)*ALIGN_16B(input_size.height)*3/2);
				buffers1[n_buffers[device_id]].start[j]= (void *)IonAlloc(buffers1[n_buffers[device_id]].length[j]);
				if (buffers1[n_buffers[device_id]].start[j] == NULL){
					 printf("Camera v4l2QueryBuf buffer allocate ERROR");
					 pthread_mutex_unlock(&mutex[device_id]);
					 return -1;
				}
				buf.m.planes[j].m.userptr = (unsigned long)buffers1[n_buffers[device_id]].start[j];
				buf.m.planes[j].length = buffers1[n_buffers[device_id]].length[j];
			//printf("buffers[%d].start[%d]=%lx\nbuf.m.planes[%d].m.userptr=%lx\n",n_buffers,j,buffers[n_buffers].start[j],j,buf.m.planes[j].m.userptr);
			//printf("Camera[%d] v4l2QueryBuf mem[%d]:0x%lx,len:%d,fd:%d,w*h:%d,%d", mCameraId,i, (unsigned long)buffers[n_buffers].start[i], mMapMem.length, mMapMem.nShareBufFd[i],mFrameWidth,mFrameHeight);
			}
			break;
		}
	}

	for (i = 0; i < n_buffers[device_id]; ++i) {
		struct v4l2_buffer buf;

		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		buf.memory = memtype;//V4L2_MEMORY_USERPTR;//V4L2_MEMORY_MMAP;
		buf.index = i;
		buf.length = nplanes;
		buf.m.planes =
		    (struct v4l2_plane *)calloc(nplanes,
						sizeof(struct v4l2_plane));
			for (int j = 0; j < nplanes; j++) {
				buf.m.planes[j].m.userptr = (unsigned long)buffers1[i].start[j];
				buf.m.planes[j].length = buffers1[i].length[j];
			}


		if (-1 == ioctl(fd[device_id], VIDIOC_QBUF, &buf)) {
			printf("VIDIOC_QBUF failed\n");
			free(buf.m.planes);
			pthread_mutex_unlock(&mutex[device_id]);
			return -1;
		}
		free(buf.m.planes);
	}
	return 0;
}

static int req_frame_buffers2(int device_id)
{
	unsigned int i;
	struct v4l2_requestbuffers req;
	struct v4l2_exportbuffer exp;

	CLEAR(req);
	req.count = req_frame_num;	// 8
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	req.memory = memtype;//V4L2_MEMORY_USERPTR;//V4L2_MEMORY_MMAP;
	if (-1 == ioctl(fd[device_id], VIDIOC_REQBUFS, &req)) {
		printf("VIDIOC_REQBUFS error\n");
		pthread_mutex_unlock(&mutex[device_id]);
		return -1;
	}

	buffers2 = (buffer *)calloc(req.count, sizeof(*buffers2));

	for (n_buffers[device_id] = 0; n_buffers[device_id] < req.count; ++n_buffers[device_id]) {
		struct v4l2_buffer buf;

		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		buf.memory = memtype;//V4L2_MEMORY_USERPTR;//V4L2_MEMORY_MMAP;
		buf.index = n_buffers[device_id];
		buf.length = nplanes;
		buf.m.planes =
		    (struct v4l2_plane *)calloc(nplanes,
						sizeof(struct v4l2_plane));
		if (buf.m.planes == NULL) {
			printf("buf.m.planes calloc failed!\n");
			pthread_mutex_unlock(&mutex[device_id]);
			return -1;
		}
		if (-1 == ioctl(fd[device_id], VIDIOC_QUERYBUF, &buf)) {
			printf("VIDIOC_QUERYBUF error\n");
			free(buf.m.planes);
			pthread_mutex_unlock(&mutex[device_id]);
			return -1;
		}
		printf("VIDIOC_QUERYBUF buf.m.planes[j].length=%d\n",buf.m.planes[0].length);
		switch (buf.memory) {
		case V4L2_MEMORY_MMAP:
			for (i = 0; i < nplanes; i++) {
				buffers2[n_buffers[device_id]].length[i] = buf.m.planes[i].length;
				buffers2[n_buffers[device_id]].start[i] =
					mmap(NULL,/* start anywhere */
					 buf.m.planes[i].length,
					 PROT_READ | PROT_WRITE,/* required */
					 MAP_SHARED, /* recommended */
					 fd[device_id], buf.m.planes[i].m.mem_offset);

				if (buffers2[n_buffers[device_id]].start[i] == MAP_FAILED) {
					printf("mmap failed\n");
					free(buf.m.planes);
					return -1;
				}
			}
			free(buf.m.planes);
			break;
		case V4L2_MEMORY_USERPTR:
		case V4L2_MEMORY_DMABUF:
			for (int j = 0; j < nplanes; j++) {
				buffers2[n_buffers[device_id]].length[j] = ALIGN_16B(ALIGN_16B(input_size.width)*ALIGN_16B(input_size.height)*3/2);
				buffers2[n_buffers[device_id]].start[j]= (void *)IonAlloc(buffers2[n_buffers[device_id]].length[j]);
				if (buffers2[n_buffers[device_id]].start[j] == NULL){
					 printf("Camera v4l2QueryBuf buffer allocate ERROR");
					 pthread_mutex_unlock(&mutex[device_id]);
					 return -1;
				}
				buf.m.planes[j].m.userptr = (unsigned long)buffers2[n_buffers[device_id]].start[j];
				buf.m.planes[j].length = buffers2[n_buffers[device_id]].length[j];
			//printf("buffers[%d].start[%d]=%lx\nbuf.m.planes[%d].m.userptr=%lx\n",n_buffers,j,buffers[n_buffers].start[j],j,buf.m.planes[j].m.userptr);
			//printf("Camera[%d] v4l2QueryBuf mem[%d]:0x%lx,len:%d,fd:%d,w*h:%d,%d", mCameraId,i, (unsigned long)buffers[n_buffers].start[i], mMapMem.length, mMapMem.nShareBufFd[i],mFrameWidth,mFrameHeight);
			}
			break;
		}
	}

	for (i = 0; i < n_buffers[device_id]; ++i) {
		struct v4l2_buffer buf;

		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		buf.memory = memtype;//V4L2_MEMORY_USERPTR;//V4L2_MEMORY_MMAP;
		buf.index = i;
		buf.length = nplanes;
		buf.m.planes =
		    (struct v4l2_plane *)calloc(nplanes,
						sizeof(struct v4l2_plane));
			for (int j = 0; j < nplanes; j++) {
				buf.m.planes[j].m.userptr = (unsigned long)buffers2[i].start[j];
				buf.m.planes[j].length = buffers2[i].length[j];
			}


		if (-1 == ioctl(fd[device_id], VIDIOC_QBUF, &buf)) {
			printf("VIDIOC_QBUF failed\n");
			free(buf.m.planes);
			pthread_mutex_unlock(&mutex[device_id]);
			return -1;
		}
		free(buf.m.planes);
	}
	return 0;
}

static int req_frame_buffers3(int device_id)
{
	unsigned int i;
	struct v4l2_requestbuffers req;
	struct v4l2_exportbuffer exp;

	CLEAR(req);
	req.count = req_frame_num;	// 8
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	req.memory = memtype;//V4L2_MEMORY_USERPTR;//V4L2_MEMORY_MMAP;
	if (-1 == ioctl(fd[device_id], VIDIOC_REQBUFS, &req)) {
		printf("VIDIOC_REQBUFS error\n");
		pthread_mutex_unlock(&mutex[device_id]);
		return -1;
	}

	buffers3 = (buffer *)calloc(req.count, sizeof(*buffers3));

	for (n_buffers[device_id] = 0; n_buffers[device_id] < req.count; ++n_buffers[device_id]) {
		struct v4l2_buffer buf;

		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		buf.memory = memtype;//V4L2_MEMORY_USERPTR;//V4L2_MEMORY_MMAP;
		buf.index = n_buffers[device_id];
		buf.length = nplanes;
		buf.m.planes =
		    (struct v4l2_plane *)calloc(nplanes,
						sizeof(struct v4l2_plane));
		if (buf.m.planes == NULL) {
			printf("buf.m.planes calloc failed!\n");
			pthread_mutex_unlock(&mutex[device_id]);
			return -1;
		}
		if (-1 == ioctl(fd[device_id], VIDIOC_QUERYBUF, &buf)) {
			printf("VIDIOC_QUERYBUF error\n");
			free(buf.m.planes);
			pthread_mutex_unlock(&mutex[device_id]);
			return -1;
		}
		printf("VIDIOC_QUERYBUF buf.m.planes[j].length=%d\n",buf.m.planes[0].length);
		switch (buf.memory) {
		case V4L2_MEMORY_MMAP:
			for (i = 0; i < nplanes; i++) {
				buffers3[n_buffers[device_id]].length[i] = buf.m.planes[i].length;
				buffers3[n_buffers[device_id]].start[i] =
					mmap(NULL,/* start anywhere */
					 buf.m.planes[i].length,
					 PROT_READ | PROT_WRITE,/* required */
					 MAP_SHARED, /* recommended */
					 fd[device_id], buf.m.planes[i].m.mem_offset);

				if (buffers3[n_buffers[device_id]].start[i] == MAP_FAILED) {
					printf("mmap failed\n");
					free(buf.m.planes);
					return -1;
				}
			}
			free(buf.m.planes);
			break;
		case V4L2_MEMORY_USERPTR:
		case V4L2_MEMORY_DMABUF:
			for (int j = 0; j < nplanes; j++) {
				buffers3[n_buffers[device_id]].length[j] = ALIGN_16B(ALIGN_16B(input_size.width)*ALIGN_16B(input_size.height)*3/2);
				buffers3[n_buffers[device_id]].start[j]= (void *)IonAlloc(buffers3[n_buffers[device_id]].length[j]);
				if (buffers3[n_buffers[device_id]].start[j] == NULL){
					 printf("Camera v4l2QueryBuf buffer allocate ERROR");
					 pthread_mutex_unlock(&mutex[device_id]);
					 return -1;
				}
				buf.m.planes[j].m.userptr = (unsigned long)buffers3[n_buffers[device_id]].start[j];
				buf.m.planes[j].length = buffers3[n_buffers[device_id]].length[j];
			//printf("buffers[%d].start[%d]=%lx\nbuf.m.planes[%d].m.userptr=%lx\n",n_buffers,j,buffers[n_buffers].start[j],j,buf.m.planes[j].m.userptr);
			//printf("Camera[%d] v4l2QueryBuf mem[%d]:0x%lx,len:%d,fd:%d,w*h:%d,%d", mCameraId,i, (unsigned long)buffers[n_buffers].start[i], mMapMem.length, mMapMem.nShareBufFd[i],mFrameWidth,mFrameHeight);
			}
			break;
		}
	}

	for (i = 0; i < n_buffers[device_id]; ++i) {
		struct v4l2_buffer buf;

		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		buf.memory = memtype;//V4L2_MEMORY_USERPTR;//V4L2_MEMORY_MMAP;
		buf.index = i;
		buf.length = nplanes;
		buf.m.planes =
		    (struct v4l2_plane *)calloc(nplanes,
						sizeof(struct v4l2_plane));
			for (int j = 0; j < nplanes; j++) {
				buf.m.planes[j].m.userptr = (unsigned long)buffers3[i].start[j];
				buf.m.planes[j].length = buffers3[i].length[j];
			}


		if (-1 == ioctl(fd[device_id], VIDIOC_QBUF, &buf)) {
			printf("VIDIOC_QBUF failed\n");
			free(buf.m.planes);
			pthread_mutex_unlock(&mutex[device_id]);
			return -1;
		}
		free(buf.m.planes);
	}
	return 0;
}

static int free_frame_buffers(int device_id)
{
	unsigned int i, j;

	memset(buffers,0,sizeof(struct buffer));
	if(device_id == 0){
		buffers = buffers0;
	}else if(device_id == 1){
		buffers = buffers1;
	}else if(device_id == 2){
		buffers = buffers2;
	}else if(device_id == 3){
		buffers = buffers3;
	}
	printf("buffers is %x !\r\n",buffers);
	for (i = 0; i < n_buffers[device_id]; ++i) {
		for (j = 0; j < nplanes; j++)
			if((V4L2_MEMORY_USERPTR == memtype )||(V4L2_MEMORY_DMABUF == memtype )){
				IonFree((void *)buffers[i].start[j]);
				buffers[i].start[j] = NULL;
			}else if(V4L2_MEMORY_MMAP == memtype ){
				if (-1 == munmap(buffers[i].start[j], buffers[i].length[j])) {
					printf("munmap error");
					return -1;
				}
			}
	}
	free(buffers);
	return 0;
}

static int camera_init(char *device_name, int sel, int mode , int device_id)
{
	struct v4l2_input inp;
	struct v4l2_streamparm parms;

	fd[device_id] = open(device_name, O_RDWR /* required */  | O_NONBLOCK, 0);

	if (fd[device_id] < 0) {
		printf("open falied %s\n",device_name);
		pthread_mutex_unlock(&mutex[device_id]);
		return -1;
	}
	printf("open %s fd[%d] = %d\n", device_name, device_id, fd[device_id]);

	inp.index = sel;
	if (-1 == ioctl(fd[device_id], VIDIOC_S_INPUT, &inp)) {
		printf("VIDIOC_S_INPUT %d error!\n", sel);
		pthread_mutex_unlock(&mutex[device_id]);
		return -1;
	}

	CLEAR(parms);
	parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	parms.parm.capture.timeperframe.numerator = 1;
	parms.parm.capture.timeperframe.denominator = fps;
	parms.parm.capture.capturemode = V4L2_MODE_VIDEO;
	/* parms.parm.capture.capturemode = V4L2_MODE_IMAGE; */
	/*when different video have the same sensor source, 1:use sensor current win, 0:find the nearest win*/
	parms.parm.capture.reserved[0] = 0;
	parms.parm.capture.reserved[1] = wdr_mode;/*2:command, 1: wdr, 0: normal*/

	if (-1 == ioctl(fd[device_id], VIDIOC_S_PARM, &parms)) {
		printf("VIDIOC_S_PARM error\n");
		pthread_mutex_unlock(&mutex[device_id]);
		return -1;
	}

	return 0;
}

static int camera_fmt_set(int mode , int device_id)
{
	struct v4l2_format fmt;

	CLEAR(fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	fmt.fmt.pix_mp.width = input_size.width;
	fmt.fmt.pix_mp.height = input_size.height;
	switch (mode) {
	case 0:
		fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_SBGGR8;
		break;
	case 1:
		fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_YUV420M;
		
		break;
	case 2:
		fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_YUV420;
		break;
	case 3:
		fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12M;
		break;
	case 4:
		fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV21;
		break;
	case 5:
		fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_SBGGR10;
		break;
	case 6:
		fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_SBGGR12;
		break;
	case 7:
		fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_SRGGB8;
		break;
	default:
		fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV21;
		break;
	}
	fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
	printf("fmt.fmt.pix_mp.pixelformat=%d\n",fmt.fmt.pix_mp.pixelformat);
	if (-1 == ioctl(fd[device_id], VIDIOC_S_FMT, &fmt)) {
		printf("VIDIOC_S_FMT error!\n");
		pthread_mutex_unlock(&mutex[device_id]);
		return -1;
	}

	if (-1 == ioctl(fd[device_id], VIDIOC_G_FMT, &fmt)) {
		printf("VIDIOC_G_FMT error!\n");
		pthread_mutex_unlock(&mutex[device_id]);
		return -1;
	} else {
		nplanes = fmt.fmt.pix_mp.num_planes;
		printf("resolution got from sensor = %d*%d num_planes = %d\n",
		       fmt.fmt.pix_mp.width, fmt.fmt.pix_mp.height,
		       fmt.fmt.pix_mp.num_planes);
	}

	return 0;
}

static int getFbWidHight(int fbid,int *w,int *h)
{
    int fd;
    char devName[20];
    memset(devName, 0 ,sizeof(char)*20 );
    sprintf(devName,"/dev/fb%d", fbid);
    struct fb_var_screeninfo var;
    struct fb_fix_screeninfo fix;
    if((fd = open(devName,O_RDWR)) == -1){
        printf("open file %s fail. \n",devName);
        return -1;
    }
    
    if(ioctl(fd,FBIOGET_VSCREENINFO,&var)==-1){
        printf("syncScreen1Thread get screen information failure\n");
        return -1;
    }
    
    if(ioctl(fd,FBIOGET_FSCREENINFO,&fix)==-1){
        printf("syncScreen1Thread get screen information failure\n");
        return -1;
    }
    close(fd);
    
    *w = var.xres;
    *h = var.yres;

	return 0;
}

void *CameraDisplay0(void *arg){
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	struct v4l2_ext_control ctrls[4];
	struct v4l2_ext_controls ext_ctrls;
	struct v4l2_control control;
	unsigned int pixformat;

	struct v4l2_buffer buf;
	char fdstr[50];
	FILE *file_fd = NULL;
	char *dst = NULL;

	int ret;
	int i, j = 0;
	int device_id = 0;

	printf("alva_test!\r\n");

#if RECODER_TEST
	V4L2BUF_t v4l2_buf;
	unsigned int cdx_memtype = MEM_TYPE_CDX_NEW;
	
	int recordwith = config_get_weith(dev_id[device_id]);
	int recordheith = config_get_heigth(dev_id[device_id]);
	
	RecordCamera *pRecordCamera = new RecordCamera(dev_id[device_id]);
	pRecordCamera->videoEncParmInit(recordwith, recordheith, 
								recordwith, recordheith, 8, 25, VENC_CODEC_H264);
								
	pRecordCamera->setCallbacks(Rec_usernotifyCallback,pRecordCamera);
	
	pRecordCamera->setDuration(1 * 60);

	pRecordCamera->start();
	pRecordCamera->startRecord();	

	dma_mem_des_t recordMem;
	memset(&recordMem, 0, sizeof(dma_mem_des_t));
	ret = allocOpen(cdx_memtype, &recordMem, NULL);  
	if (ret < 0) {
		LOGE("ion_alloc_open failed");
	}
	recordMem.size = recordwith*recordheith*3/2;
	ret = allocAlloc(cdx_memtype, &recordMem, NULL);
	if(ret < 0){
		ALOGE("allocAlloc failed");
	}
#endif

	pthread_mutex_lock(&mutex[0]);

	if (-1 == camera_init(dev_addr[device_id], 0, 4 ,device_id))
		return (void *)-1;
	if (-1 == camera_fmt_set(4,device_id))
		return (void *)-1;	
	if (-1 == req_frame_buffers0(device_id))
		return (void *)-1;
	
#ifdef DMAFD_TEST
	// alloc ion mem to let gpu to use
	unsigned int pic_size = 1280*720*3/2;
	for (int i = 0; i < DISP_ION_NUM; i++) {
		gDispTab0[i].len = pic_size;
		gDispTab0[i].vir = (void *)IonAlloc(pic_size);
		gDispTab0[i].dmafd = IonVir2fd(gDispTab0[i].vir);
	}
#if 0
	if(0 == getFbWidHight(0,&egl_param[0],&egl_param[1]) ){
	}else{
		egl_param[0] = 1280;
		egl_param[1] = 720;
	}

	egl_param[2] = 1280;
	egl_param[3] = 720;

	create_textture((void *)&egl_param);
#endif
#endif

	if (-1 == ioctl(fd[device_id], VIDIOC_STREAMON, &type)) {
		printf("VIDIOC_STREAMON failed\n");
		pthread_mutex_unlock(&mutex[0]);
		return (void *)-1;
	} else
		printf("VIDIOC_STREAMON ok\n");
		pthread_mutex_unlock(&mutex[0]);
	while (1) {
			pthread_cond_signal(&cond[0]);

			fd_set fds;
			struct timeval tv;
			int r;

			FD_ZERO(&fds);
			FD_SET(fd[device_id], &fds);

			tv.tv_sec = 2; /* Timeout. */
			tv.tv_usec = 0;

			r = select(fd[device_id] + 1, &fds, NULL, NULL, &tv);

			if (-1 == r) {
				if (errno == EINTR)
					continue;
				printf("select err\n");
			}
			if (r == 0) {
				fprintf(stderr, "select timeout\n");
				continue;
			}

			CLEAR(buf);
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
			buf.memory = memtype;//V4L2_MEMORY_USERPTR;//V4L2_MEMORY_MMAP;
			buf.length = nplanes;
			buf.m.planes =
	    		(struct v4l2_plane *)calloc(nplanes, sizeof(struct v4l2_plane));

			if (-1 == ioctl(fd[device_id], VIDIOC_DQBUF, &buf)) {
				free(buf.m.planes);
				printf("VIDIOC_DQBUF failed\n");
				return (void *)-1;
			}

			assert(buf.index < n_buffers[device_id]);

#if DMAFD_TEST
		int dmafd;
		//if u dont want memcpy,use reference count instead to avoid camera driver use ion when gpu using
		memcpy(gDispTab0[buf.index].vir,(void *)buffers0[buf.index].start[0],gDispTab0[buf.index].len);
		IonFlushCache(gDispTab0[buf.index].vir,gDispTab0[buf.index].len );
		//normal mode,eend camera data to gpu in 33ms
//				info_aframe(&gDispTab[buf.index].dmafd);

		vb_dma[0] = gDispTab0[buf.index].dmafd;
		elg_Image[0] = bind_dmafd(vb_dma[0], egl_display);
#endif
#if RECODER_TEST
		memcpy((void *)recordMem.vir,(void *)buffers0[buf.index].start[0],gDispTab0[buf.index].len);
		flushCache(cdx_memtype, &recordMem, NULL);
		v4l2_buf.width = 1280;
		v4l2_buf.height = 720;
		v4l2_buf.addrPhyY = recordMem.phy;
		v4l2_buf.addrPhyC = recordMem.phy + v4l2_buf.width*v4l2_buf.height;
		v4l2_buf.addrVirY = recordMem.vir;
		v4l2_buf.addrVirC = recordMem.vir + v4l2_buf.width*v4l2_buf.height;
		v4l2_buf.dmafd = recordMem.ion_buffer.fd_data.aw_fd;
		v4l2_buf.timeStamp =  (int64_t) systemTime();	
		pRecordCamera->dataCallbackTimestamp(v4l2_buf.timeStamp,CAMERA_MSG_VIDEO_FRAME, 
									(char *)&v4l2_buf, NULL);	
#endif
		if (-1 == ioctl(fd[device_id], VIDIOC_QBUF, &buf)) {
			printf("VIDIOC_QBUF buf.index %d failed\n", buf.index);
			free(buf.m.planes);
			return (void *)-1;
		}

		free(buf.m.planes);
	}
	pthread_exit(NULL);
	disp_disable();
	usleep(20*1000);
#if RECODER_TEST

	allocFree(cdx_memtype, &recordMem, NULL);
    pRecordCamera->stopRecord();
    pRecordCamera->stop();
    pRecordCamera->videoEncDeinit();
    delete pRecordCamera;
    pRecordCamera = NULL;

	printf("record run finish\n");
#endif
	if (-1 == ioctl(fd[device_id], VIDIOC_STREAMOFF, &type)) {
		printf("VIDIOC_STREAMOFF failed\n");
		return (void *)-1;
	} else
		printf("VIDIOC_STREAMOFF ok\n");

	if (-1 == free_frame_buffers(device_id))
		return (void *)-1;
}

void *CameraDisplay1(void *arg){
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	struct v4l2_ext_control ctrls[4];
	struct v4l2_ext_controls ext_ctrls;
	struct v4l2_control control;
	unsigned int pixformat;

	struct v4l2_buffer buf;
	char fdstr[50];
	FILE *file_fd = NULL;
	char *dst = NULL;

	int ret;
	int i, j = 0;
	int device_id = 1;

	printf("alva_test!\r\n");
#if RECODER_TEST
	V4L2BUF_t v4l2_buf;
	unsigned int cdx_memtype = MEM_TYPE_CDX_NEW;
	
	int recordwith = config_get_weith(dev_id[device_id]);
	int recordheith = config_get_heigth(dev_id[device_id]);
	
	RecordCamera *pRecordCamera = new RecordCamera(dev_id[device_id]);
	pRecordCamera->videoEncParmInit(recordwith, recordheith, 
								recordwith, recordheith, 8, 25, VENC_CODEC_H264);
								
	pRecordCamera->setCallbacks(Rec_usernotifyCallback,pRecordCamera);
	
	pRecordCamera->setDuration(1 * 60);

	pRecordCamera->start();
	pRecordCamera->startRecord();	

	dma_mem_des_t recordMem;
	memset(&recordMem, 0, sizeof(dma_mem_des_t));
	ret = allocOpen(cdx_memtype, &recordMem, NULL);  
	if (ret < 0) {
		LOGE("ion_alloc_open failed");
	}
	recordMem.size = recordwith*recordheith*3/2;
	ret = allocAlloc(cdx_memtype, &recordMem, NULL);
	if(ret < 0){
		ALOGE("allocAlloc failed");
	}
#endif
	pthread_mutex_lock(&mutex[1]);
	if (-1 == camera_init(dev_addr[device_id], 0, 4 ,device_id))
		return (void *)-1;
	if (-1 == camera_fmt_set(4,device_id))
		return (void *)-1;	
	if (-1 == req_frame_buffers1(device_id))
		return (void *)-1;
	
#ifdef DMAFD_TEST
	// alloc ion mem to let gpu to use
	unsigned int pic_size = 1280*720*3/2;
	for (int i = 0; i < DISP_ION_NUM; i++) {
		gDispTab1[i].len = pic_size;
		gDispTab1[i].vir = (void *)IonAlloc(pic_size);
		gDispTab1[i].dmafd = IonVir2fd(gDispTab1[i].vir);
	}
#if 0
	if(0 == getFbWidHight(0,&egl_param[0],&egl_param[1]) ){
	}else{
		egl_param[0] = 1280;
		egl_param[1] = 720;
	}

	egl_param[2] = 1280;
	egl_param[3] = 720;

	create_textture((void *)&egl_param);
#endif
#endif

	if (-1 == ioctl(fd[device_id], VIDIOC_STREAMON, &type)) {
		printf("VIDIOC_STREAMON failed\n");
		pthread_mutex_unlock(&mutex[1]);
		return (void *)-1;
	} else
		printf("VIDIOC_STREAMON ok\n");
		pthread_mutex_unlock(&mutex[1]);
		
	while (1) {
			pthread_cond_signal(&cond[1]);
			fd_set fds;
			struct timeval tv;
			int r;

			FD_ZERO(&fds);
			FD_SET(fd[device_id], &fds);

			tv.tv_sec = 2; /* Timeout. */
			tv.tv_usec = 0;

			r = select(fd[device_id] + 1, &fds, NULL, NULL, &tv);

			if (-1 == r) {
				if (errno == EINTR)
					continue;
				printf("select err\n");
			}
			if (r == 0) {
				fprintf(stderr, "select timeout\n");
				continue;
			}

			CLEAR(buf);
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
			buf.memory = memtype;//V4L2_MEMORY_USERPTR;//V4L2_MEMORY_MMAP;
			buf.length = nplanes;
			buf.m.planes =
	    		(struct v4l2_plane *)calloc(nplanes, sizeof(struct v4l2_plane));

			if (-1 == ioctl(fd[device_id], VIDIOC_DQBUF, &buf)) {
				free(buf.m.planes);
				printf("VIDIOC_DQBUF failed\n");
				return (void *)-1;
			}

			assert(buf.index < n_buffers[device_id]);

#if DMAFD_TEST
		int dmafd;
		//if u dont want memcpy,use reference count instead to avoid camera driver use ion when gpu using
		memcpy(gDispTab1[buf.index].vir,(void *)buffers1[buf.index].start[0],gDispTab1[buf.index].len);
		IonFlushCache(gDispTab1[buf.index].vir,gDispTab1[buf.index].len );
		//normal mode,eend camera data to gpu in 33ms
//				info_aframe(&gDispTab[buf.index].dmafd);

		vb_dma[1] = gDispTab1[buf.index].dmafd;
		elg_Image[1] = bind_dmafd(vb_dma[1], egl_display);

#endif
#if RECODER_TEST
		memcpy((void *)recordMem.vir,(void *)buffers0[buf.index].start[0],gDispTab0[buf.index].len);
		flushCache(cdx_memtype, &recordMem, NULL);
		v4l2_buf.width = 1280;
		v4l2_buf.height = 720;
		v4l2_buf.addrPhyY = recordMem.phy;
		v4l2_buf.addrPhyC = recordMem.phy + v4l2_buf.width*v4l2_buf.height;
		v4l2_buf.addrVirY = recordMem.vir;
		v4l2_buf.addrVirC = recordMem.vir + v4l2_buf.width*v4l2_buf.height;
		v4l2_buf.dmafd = recordMem.ion_buffer.fd_data.aw_fd;
		v4l2_buf.timeStamp =  (int64_t) systemTime();	
		pRecordCamera->dataCallbackTimestamp(v4l2_buf.timeStamp,CAMERA_MSG_VIDEO_FRAME, 
									(char *)&v4l2_buf, NULL);	
#endif

		if (-1 == ioctl(fd[device_id], VIDIOC_QBUF, &buf)) {
			printf("VIDIOC_QBUF buf.index %d failed\n", buf.index);
			free(buf.m.planes);
			return (void *)-1;
		}

		free(buf.m.planes);
	}
	pthread_exit(NULL);
	disp_disable();
	usleep(20*1000);
#if RECODER_TEST

	allocFree(cdx_memtype, &recordMem, NULL);
    pRecordCamera->stopRecord();
    pRecordCamera->stop();
    pRecordCamera->videoEncDeinit();
    delete pRecordCamera;
    pRecordCamera = NULL;

	printf("record run finish\n");
#endif

	if (-1 == ioctl(fd[device_id], VIDIOC_STREAMOFF, &type)) {
		printf("VIDIOC_STREAMOFF failed\n");
		return (void *)-1;
	} else
		printf("VIDIOC_STREAMOFF ok\n");

	if (-1 == free_frame_buffers(device_id))
		return (void *)-1;
}

void *CameraDisplay2(void *arg){
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	struct v4l2_ext_control ctrls[4];
	struct v4l2_ext_controls ext_ctrls;
	struct v4l2_control control;
	unsigned int pixformat;

	struct v4l2_buffer buf;
	char fdstr[50];
	FILE *file_fd = NULL;
	char *dst = NULL;

	int ret;
	int i, j = 0;
	int device_id = 2;

	printf("alva_test!\r\n");
#if RECODER_TEST
	V4L2BUF_t v4l2_buf;
	unsigned int cdx_memtype = MEM_TYPE_CDX_NEW;
	
	int recordwith = config_get_weith(dev_id[device_id]);
	int recordheith = config_get_heigth(dev_id[device_id]);
	
	RecordCamera *pRecordCamera = new RecordCamera(dev_id[device_id]);
	pRecordCamera->videoEncParmInit(recordwith, recordheith, 
								recordwith, recordheith, 8, 25, VENC_CODEC_H264);
								
	pRecordCamera->setCallbacks(Rec_usernotifyCallback,pRecordCamera);
	
	pRecordCamera->setDuration(1 * 60);

	pRecordCamera->start();
	pRecordCamera->startRecord();	

	dma_mem_des_t recordMem;
	memset(&recordMem, 0, sizeof(dma_mem_des_t));
	ret = allocOpen(cdx_memtype, &recordMem, NULL);  
	if (ret < 0) {
		LOGE("ion_alloc_open failed");
	}
	recordMem.size = recordwith*recordheith*3/2;
	ret = allocAlloc(cdx_memtype, &recordMem, NULL);
	if(ret < 0){
		ALOGE("allocAlloc failed");
	}
#endif
	pthread_mutex_lock(&mutex[2]);
	if (-1 == camera_init(dev_addr[device_id], 0, 4 ,device_id))
		return (void *)-1;
	if (-1 == camera_fmt_set(4,device_id))
		return (void *)-1;	
	if (-1 == req_frame_buffers2(device_id))
		return (void *)-1;
	
#ifdef DMAFD_TEST
	// alloc ion mem to let gpu to use
	unsigned int pic_size = 1280*720*3/2;
	for (int i = 0; i < DISP_ION_NUM; i++) {
		gDispTab2[i].len = pic_size;
		gDispTab2[i].vir = (void *)IonAlloc(pic_size);
		gDispTab2[i].dmafd = IonVir2fd(gDispTab2[i].vir);
	}
#if 0
	if(0 == getFbWidHight(0,&egl_param[0],&egl_param[1]) ){
	}else{
		egl_param[0] = 1280;
		egl_param[1] = 720;
	}

	egl_param[2] = 1280;
	egl_param[3] = 720;

	create_textture((void *)&egl_param);
#endif
#endif

	if (-1 == ioctl(fd[device_id], VIDIOC_STREAMON, &type)) {
		printf("VIDIOC_STREAMON failed\n");
		pthread_mutex_unlock(&mutex[2]);
		return (void *)-1;
	} else
		printf("VIDIOC_STREAMON ok\n");
		pthread_mutex_unlock(&mutex[2]);
		
	while (1) {
			pthread_cond_signal(&cond[2]);
			fd_set fds;
			struct timeval tv;
			int r;

			FD_ZERO(&fds);
			FD_SET(fd[device_id], &fds);

			tv.tv_sec = 2; /* Timeout. */
			tv.tv_usec = 0;

			r = select(fd[device_id] + 1, &fds, NULL, NULL, &tv);

			if (-1 == r) {
				if (errno == EINTR)
					continue;
				printf("select err\n");
			}
			if (r == 0) {
				fprintf(stderr, "select timeout\n");
				continue;
			}

			CLEAR(buf);
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
			buf.memory = memtype;//V4L2_MEMORY_USERPTR;//V4L2_MEMORY_MMAP;
			buf.length = nplanes;
			buf.m.planes =
	    		(struct v4l2_plane *)calloc(nplanes, sizeof(struct v4l2_plane));

			if (-1 == ioctl(fd[device_id], VIDIOC_DQBUF, &buf)) {
				free(buf.m.planes);
				printf("VIDIOC_DQBUF failed\n");
				return (void *)-1;
			}

			assert(buf.index < n_buffers[device_id]);

#if DMAFD_TEST
		int dmafd;
		//if u dont want memcpy,use reference count instead to avoid camera driver use ion when gpu using
		memcpy(gDispTab2[buf.index].vir,(void *)buffers2[buf.index].start[0],gDispTab2[buf.index].len);
		IonFlushCache(gDispTab2[buf.index].vir,gDispTab2[buf.index].len );
		//normal mode,eend camera data to gpu in 33ms
//				info_aframe(&gDispTab[buf.index].dmafd);

		vb_dma[2] = gDispTab2[buf.index].dmafd;
		elg_Image[2] = bind_dmafd(vb_dma[2], egl_display);


#endif
#if RECODER_TEST
		memcpy((void *)recordMem.vir,(void *)buffers0[buf.index].start[0],gDispTab0[buf.index].len);
		flushCache(cdx_memtype, &recordMem, NULL);
		v4l2_buf.width = 1280;
		v4l2_buf.height = 720;
		v4l2_buf.addrPhyY = recordMem.phy;
		v4l2_buf.addrPhyC = recordMem.phy + v4l2_buf.width*v4l2_buf.height;
		v4l2_buf.addrVirY = recordMem.vir;
		v4l2_buf.addrVirC = recordMem.vir + v4l2_buf.width*v4l2_buf.height;
		v4l2_buf.dmafd = recordMem.ion_buffer.fd_data.aw_fd;
		v4l2_buf.timeStamp =  (int64_t) systemTime();	
		pRecordCamera->dataCallbackTimestamp(v4l2_buf.timeStamp,CAMERA_MSG_VIDEO_FRAME, 
									(char *)&v4l2_buf, NULL);	
#endif

		if (-1 == ioctl(fd[device_id], VIDIOC_QBUF, &buf)) {
			printf("VIDIOC_QBUF buf.index %d failed\n", buf.index);
			free(buf.m.planes);
			return (void *)-1;
		}

		free(buf.m.planes);
	}
	pthread_exit(NULL);
	disp_disable();
	usleep(20*1000);
#if RECODER_TEST

	allocFree(cdx_memtype, &recordMem, NULL);
    pRecordCamera->stopRecord();
    pRecordCamera->stop();
    pRecordCamera->videoEncDeinit();
    delete pRecordCamera;
    pRecordCamera = NULL;

	printf("record run finish\n");
#endif

	if (-1 == ioctl(fd[device_id], VIDIOC_STREAMOFF, &type)) {
		printf("VIDIOC_STREAMOFF failed\n");
		return (void *)-1;
	} else
		printf("VIDIOC_STREAMOFF ok\n");

	if (-1 == free_frame_buffers(device_id))
		return (void *)-1;
}

void *CameraDisplay3(void *arg){
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	struct v4l2_ext_control ctrls[4];
	struct v4l2_ext_controls ext_ctrls;
	struct v4l2_control control;
	unsigned int pixformat;

	struct v4l2_buffer buf;
	char fdstr[50];
	FILE *file_fd = NULL;
	char *dst = NULL;

	int ret;
	int i, j = 0;
	int device_id = 3;

	printf("alva_test!\r\n");
#if RECODER_TEST
	V4L2BUF_t v4l2_buf;
	unsigned int cdx_memtype = MEM_TYPE_CDX_NEW;
	
	int recordwith = config_get_weith(dev_id[device_id]);
	int recordheith = config_get_heigth(dev_id[device_id]);
	
	RecordCamera *pRecordCamera = new RecordCamera(dev_id[device_id]);
	pRecordCamera->videoEncParmInit(recordwith, recordheith, 
								recordwith, recordheith, 8, 25, VENC_CODEC_H264);
								
	pRecordCamera->setCallbacks(Rec_usernotifyCallback,pRecordCamera);
	
	pRecordCamera->setDuration(1 * 60);

	pRecordCamera->start();
	pRecordCamera->startRecord();	

	dma_mem_des_t recordMem;
	memset(&recordMem, 0, sizeof(dma_mem_des_t));
	ret = allocOpen(cdx_memtype, &recordMem, NULL);  
	if (ret < 0) {
		LOGE("ion_alloc_open failed");
	}
	recordMem.size = recordwith*recordheith*3/2;
	ret = allocAlloc(cdx_memtype, &recordMem, NULL);
	if(ret < 0){
		ALOGE("allocAlloc failed");
	}
#endif
	pthread_mutex_lock(&mutex[3]);
	if (-1 == camera_init(dev_addr[device_id], 0, 4 ,device_id))
		return (void *)-1;
	if (-1 == camera_fmt_set(4,device_id))
		return (void *)-1;	
	if (-1 == req_frame_buffers3(device_id))
		return (void *)-1;
	
#ifdef DMAFD_TEST
	// alloc ion mem to let gpu to use
	unsigned int pic_size = 1280*720*3/2;
	for (int i = 0; i < DISP_ION_NUM; i++) {
		gDispTab3[i].len = pic_size;
		gDispTab3[i].vir = (void *)IonAlloc(pic_size);
		gDispTab3[i].dmafd = IonVir2fd(gDispTab3[i].vir);
	}
#if 0
	if(0 == getFbWidHight(0,&egl_param[0],&egl_param[1]) ){
	}else{
		egl_param[0] = 1280;
		egl_param[1] = 720;
	}

	egl_param[2] = 1280;
	egl_param[3] = 720;

	create_textture((void *)&egl_param);
#endif
#endif

	if (-1 == ioctl(fd[device_id], VIDIOC_STREAMON, &type)) {
		printf("VIDIOC_STREAMON failed\n");
		pthread_mutex_unlock(&mutex[3]);
		return (void *)-1;
	} else
		printf("VIDIOC_STREAMON ok\n");
		pthread_mutex_unlock(&mutex[3]);
		
	while (1) {
			pthread_cond_signal(&cond[3]);
			fd_set fds;
			struct timeval tv;
			int r;

			FD_ZERO(&fds);
			FD_SET(fd[device_id], &fds);

			tv.tv_sec = 2; /* Timeout. */
			tv.tv_usec = 0;

			r = select(fd[device_id] + 1, &fds, NULL, NULL, &tv);

			if (-1 == r) {
				if (errno == EINTR)
					continue;
				printf("select err\n");
			}
			if (r == 0) {
				fprintf(stderr, "select timeout\n");
				continue;
			}

			CLEAR(buf);
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
			buf.memory = memtype;//V4L2_MEMORY_USERPTR;//V4L2_MEMORY_MMAP;
			buf.length = nplanes;
			buf.m.planes =
	    		(struct v4l2_plane *)calloc(nplanes, sizeof(struct v4l2_plane));

			if (-1 == ioctl(fd[device_id], VIDIOC_DQBUF, &buf)) {
				free(buf.m.planes);
				printf("VIDIOC_DQBUF failed\n");
				return (void *)-1;
			}

			assert(buf.index < n_buffers[device_id]);

#if DMAFD_TEST
		int dmafd;
		//if u dont want memcpy,use reference count instead to avoid camera driver use ion when gpu using
		memcpy(gDispTab3[buf.index].vir,(void *)buffers3[buf.index].start[0],gDispTab3[buf.index].len);
		IonFlushCache(gDispTab3[buf.index].vir,gDispTab3[buf.index].len );
		//normal mode,eend camera data to gpu in 33ms
//				info_aframe(&gDispTab[buf.index].dmafd);

		vb_dma[3] = gDispTab3[buf.index].dmafd;
		elg_Image[3] = bind_dmafd(vb_dma[3], egl_display);


#endif
#if RECODER_TEST
		memcpy((void *)recordMem.vir,(void *)buffers0[buf.index].start[0],gDispTab0[buf.index].len);
		flushCache(cdx_memtype, &recordMem, NULL);
		v4l2_buf.width = 1280;
		v4l2_buf.height = 720;
		v4l2_buf.addrPhyY = recordMem.phy;
		v4l2_buf.addrPhyC = recordMem.phy + v4l2_buf.width*v4l2_buf.height;
		v4l2_buf.addrVirY = recordMem.vir;
		v4l2_buf.addrVirC = recordMem.vir + v4l2_buf.width*v4l2_buf.height;
		v4l2_buf.dmafd = recordMem.ion_buffer.fd_data.aw_fd;
		v4l2_buf.timeStamp =  (int64_t) systemTime();	
		pRecordCamera->dataCallbackTimestamp(v4l2_buf.timeStamp,CAMERA_MSG_VIDEO_FRAME, 
									(char *)&v4l2_buf, NULL);	
#endif

		if (-1 == ioctl(fd[device_id], VIDIOC_QBUF, &buf)) {
			printf("VIDIOC_QBUF buf.index %d failed\n", buf.index);
			free(buf.m.planes);
			return (void *)-1;
		}

		free(buf.m.planes);
	}
	pthread_exit(NULL);
	disp_disable();
	usleep(20*1000);
#if RECODER_TEST

	allocFree(cdx_memtype, &recordMem, NULL);
    pRecordCamera->stopRecord();
    pRecordCamera->stop();
    pRecordCamera->videoEncDeinit();
    delete pRecordCamera;
    pRecordCamera = NULL;

	printf("record run finish\n");
#endif

	if (-1 == ioctl(fd[device_id], VIDIOC_STREAMOFF, &type)) {
		printf("VIDIOC_STREAMOFF failed\n");
		return (void *)-1;
	} else
		printf("VIDIOC_STREAMOFF ok\n");

	if (-1 == free_frame_buffers(device_id))
		return (void *)-1;
}

#define TEST_WIDTH 1280 // alva add
#define TEST_HEIGHT 720

int* phrase_input(int argc,char *argv[]){
//	static int input_dev[4];
	if(argc != 5){
		printf("input error!");
		return (int *)1;
	}else if(atoi(argv[1]) > 8 || atoi(argv[2]) >8 || atoi(argv[3]) >8 || atoi(argv[4]) >8){
		printf("dev is not found!\r\n");
		return (int *)1;
	}

	for(int i =0 ;i<4;i++){
		dev_id[i] = atoi(argv[i+1]);
	}

	return dev_id;
}

void init_cameradev(int *arg){	
	sprintf(dev_name0,"/dev/video%d",arg[0]);
	dev_addr[0] = dev_name0;
	sprintf(dev_name1,"/dev/video%d",arg[1]);
	dev_addr[1] = dev_name1;
	sprintf(dev_name2,"/dev/video%d",arg[2]);
	dev_addr[2] = dev_name2;
	sprintf(dev_name3,"/dev/video%d",arg[3]);
	dev_addr[3] = dev_name3;
}

int main(int argc, char *argv[])
{
	int i;
	int sel = 0;
	int width = 640;
	int height = 480;
	int mode = 1;
	struct timeval tv1, tv2;
	float tv;

	install_sig_handler();
	init_eglimg_dmafd();
	init_cameradev(phrase_input(argc,argv));

#if RECODER_TEST
	sdk_log_setlevel(6);
	DvrRecordManagerInit(); 
#endif

	IonAllocOpen();
	input_size.width = TEST_WIDTH;
	input_size.height = TEST_WIDTH;

//	xcb_avmlib_Window_Init(1280, 800, egl_display, egl_surface);//显示窗口初始化 屏幕大小1280*480

#if 1
	usleep(30000);
	pthread_create(&tid360Camera0[0], NULL, CameraDisplay0, NULL);
	usleep(30000);
	pthread_create(&tid360Camera1[0], NULL, CameraDisplay1, NULL);
	usleep(30000);
	pthread_create(&tid360Camera2[0], NULL, CameraDisplay2, NULL);
	usleep(30000);
	pthread_create(&tid360Camera3[0], NULL, CameraDisplay3, NULL);
	usleep(30000);
#endif

	pthread_create(&avm_camera,NULL,AVM_Camera,NULL);

#if 1
	pthread_join(tid360Camera0[0], NULL);
	pthread_join(tid360Camera1[0], NULL);
	pthread_join(tid360Camera2[0], NULL);
	pthread_join(tid360Camera3[0], NULL);
#endif	
	while (0) {
		usleep(30000);
	}

	return 0;
}
