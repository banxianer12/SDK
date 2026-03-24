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
#include "posixTimer.h"


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

timer_t  mTimerID;

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

void __avm_timer_cb(sigval_t sig);

volatile int camera_run=0;

volatile  int camera1=0;
volatile  int camera2=0;
volatile  int camera3=0;
volatile  int camera4=0;

volatile int avm_run=0;


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

void read_img_test();
void mod_ui_porcess();
void bsd_ui_process();	
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
struct KeyState
{
  uint8_t mKbKey;
  uint8_t mKbMode;
};

struct KeyState Keystate;

#define KEYVAL 0xa

int iKey = KEYVAL;

int key = 0;
/********************************************************************************/

static int KeyDown()
{
  struct timeval lTv;
  fd_set lFdSet;
  lTv.tv_sec = 0;
  lTv.tv_usec = 50;

  FD_ZERO(&lFdSet);

  FD_SET(STDIN_FILENO, &lFdSet);
  select(STDIN_FILENO + 1, &lFdSet, NULL, NULL, &lTv);

  return FD_ISSET(STDIN_FILENO, &lFdSet);
} // KeyDown()

static int GetCharNonBlock()
{
  int lChar = EOF;

  usleep(0.5);
  if (KeyDown())
  {
    lChar = fgetc(stdin);
  }
  return lChar;
}

static char GetChar()
{
  int lChar = GetCharNonBlock();
  return (char)((lChar < 0) ? 0 : lChar & 0xff);
}

/*********************************************************************************
  *Function:  		KbAction
  * Description：  
  *Input:  struct KeyState Keystate
  *Output:  nc
  *Return: void 
  *Others: nc
**********************************************************************************/
static void KbAction(struct KeyState Keystate)
{
  uint8_t lKey = Keystate.mKbKey;

  if (lKey == '0')
  {
    iKey = 0;
    return;
  }
  if (lKey == '1')
  {
    iKey = 1;
    return;
  }

  if (lKey == '2')
  {
    iKey = 2;
    return;
  }
  if (lKey == '3')
  {
    iKey = 3;
    return;
  }

  if (lKey == '4')
  {
    iKey = 4;
    return;
  }

  if (lKey == '5')
  {
    iKey = 5;
    return;
  }
  if (lKey == '6')
  {
    iKey = 6;
    return;
  }
  if (lKey == '7')
  {
    iKey = 7;
    return;
  }
  if (lKey == '8')
  {
    iKey = 8;
    return;
  }
  if (lKey == '9')
  {
    iKey = 9;
    return;
  }
  if (lKey == 'a')
  {
    iKey = 0x0a;
    return;
  }
  if (lKey == 'b')
  {
    iKey = 0x0b;
    return;
  }
  if (lKey == 'c')
  {
    iKey = 0x0c;
    return;
  }
  if (lKey == 'd')
  {
    iKey = 0x0d;
    return;
  }
  if (lKey == 'e')
  {
    iKey = 0x0e;
    return;
  }

  if (lKey == 'q')
  {
    iKey = 0xff;
    printf("Press key q, select iKey  q out\n");
    return;
  }
}

void KeyboardInputProcess(struct KeyState Keystate)
{

  Keystate.mKbKey = GetChar();

  if (Keystate.mKbKey)
  {
    KbAction(Keystate);
  }
  Keystate.mKbKey = 0;
}

//=================================================================


void car_model_init()
{
  xcb_avmlib_car_Init("./gpu_data_default/avm_license", &avm_car_Handle, (char *)"./car0");
  printf("car end\n");
  printf("==================1\n");
  avm_car_Handle.blind_2d->blind_area_width = 2050;
  avm_car_Handle.blind_2d->blind_area_length = 5000;
  avm_car_Handle.blind_2d->blind_area_cx = 0;
  avm_car_Handle.blind_2d->blind_area_cy = 0;
  printf("==================2\n");
  avm_car_Handle.car_3d.wheel.wheel_axis[0][0] = 0; //*2100/srcw
  avm_car_Handle.car_3d.wheel.wheel_axis[0][1] = 0; //*5250/srcl
  avm_car_Handle.car_3d.wheel.wheel_axis[0][2] = 0; //*1550/srch
  avm_car_Handle.car_3d.wheel.wheel_axis[1][0] = 0;
  avm_car_Handle.car_3d.wheel.wheel_axis[1][1] = 0;
  avm_car_Handle.car_3d.wheel.wheel_axis[1][2] = 0;
  avm_car_Handle.car_3d.wheel.wheel_axis[2][0] = 0;
  avm_car_Handle.car_3d.wheel.wheel_axis[2][1] = 0;
  avm_car_Handle.car_3d.wheel.wheel_axis[2][2] = 0;
  avm_car_Handle.car_3d.wheel.wheel_axis[3][0] = 0;
  avm_car_Handle.car_3d.wheel.wheel_axis[3][1] = 0;
  avm_car_Handle.car_3d.wheel.wheel_axis[3][2] = 0;
  avm_car_Handle.car_3d.wheel.swing = true;
  avm_car_Handle.car_3d.wheel.swing_angle = 0;
  avm_car_Handle.car_3d.wheel.roll = true;
  avm_car_Handle.car_3d.wheel.roll_angle = 0;
  

  float light[3];
  light[0] = 1.0;
  light[1] = 1.0;
  light[2] = 1.0;
  xcb_avmlib_car_SetAmbientLight(light);
  avm_car_Handle.iCarWidth = 2000;
  avm_car_Handle.iCarLength = 5180;
  avm_car_Handle.iBlindAreaWidth = 2100;
  avm_car_Handle.iBlindAreaLength = 5200;

  float srcw, srcl, srch;
  xcb_avmlib_3DCarmodel_Resize(NULL, NULL, srcw, srcl, srch, 2100, 5250,1550);
  printf("ow: %f; ol: %f; oh: %f\n", srcw, srcl, srch);

}
//=================================================================

void load_2d_3d_bowModle()
{

xcb_avmlib_stitching_Init(&avm_stitching_Handle, "./gpu_data_default/avm_license");//

xcb_avmlib_stitching_Gen_VBO(NULL, "./gpu_data_default/BowlModel_2D",
                               &avm_stitching_Handle, 0);

xcb_avmlib_stitching_Gen_VBO(NULL, "./gpu_data_default/BowlModel",
                               &avm_stitching_Handle, 1);
}

void Render_2D(bool show_fround_view)
{
	fGreyRatio[0] = 1;
	fGreyRatio[1] = 1;
	fGreyRatio[2] = 1;
	fGreyRatio[3] = 1;

	camera_state.camera_state[0] = true;
	camera_state.camera_state[1] = true;
	camera_state.camera_state[2] = true;
	camera_state.camera_state[3] = true;
	xcb_avmlib_car_2d_Set_ViewParameter(cameraParameter, 0, 1, 0);
	xcb_avmlib_stitching_TopView_Render(&avm_stitching_Handle, cameraParameter, &camera_state, texture_id, 0, &fGreyRatio[0]);

	if(!show_fround_view)
   	{
		xcb_avmlib_car_Render_bottom(&avm_car_Handle, cameraParameter);
		xcb_avmlib_car_Render_2d(&avm_car_Handle, cameraParameter);
	}
	else
	{
        xcb_avm_fbo_transchassis_Texture_Renderwithviewpara2D(&avm_fbo_transchassis,
                                                              cameraParameter,
                                                              &pTexResult);
	}
	
    xcb_avm_line_dynamic_bird_Render(&avmLineDynamicBirdHandle0);//画俯视图轨迹线
	xcb_avm_line_dynamic_bird_Render(&avmLineDynamicBirdHandle1);//画俯视图轨迹线
	xcb_avm_line_dynamic_bird_Render(&avmLineDynamicBirdHandle2);//画俯视图轨迹线
	
}

void Render_3D(bool use_ground_view)
{
	fGreyRatio[0] = 1;
	fGreyRatio[1] = 1;
	fGreyRatio[2] = 1;
	fGreyRatio[3] = 1;

	camera_state.camera_state[0] = true;
	camera_state.camera_state[1] = true;
	camera_state.camera_state[2] = true;
	camera_state.camera_state[3] = true;
	xcb_avmlib_stitching_TopView_Render(&avm_stitching_Handle, cameraParameter3D,
										&camera_state, texture_id, 1, &fGreyRatio[0]);

	if(!use_ground_view)
   	{
		xcb_avmlib_car_Render_bottom(&avm_car_Handle, cameraParameter3D);
		xcb_avmlib_car_Render_3d(&avm_car_Handle, cameraParameter3D);
	}
	else
	{
        xcb_avm_fbo_transchassis_Texture_Renderwithviewpara3D(&avm_fbo_transchassis,
                                                              cameraParameter3D,
                                                              &pTexResult);
	}

}

void render_single(View_name name)
{
	
	switch (name)
	{
	case or_f:
	{
		avm_single_Handle.view = or_f;
		avmLineDynamicSingleHandle.line_dynamic_single->single.isResived=false;
		avmLineDynamicSingleHandle.line_dynamic_single->type=0;
		if(avmLineDynamicSingleHandle.line_dynamic_single->fAngle>0)
		{
		avmLineDynamicSingleHandle.bDemoorNot[0] = true; //每三根线为一组，对应的标记为true，则显示，否则不显示;
		avmLineDynamicSingleHandle.bDemoorNot[1] = false;
		avmLineDynamicSingleHandle.bDemoorNot[2] = false;
		avmLineDynamicSingleHandle.bDemoorNot[3] = false;
		}else
		{
		avmLineDynamicSingleHandle.bDemoorNot[0] = false; //每三根线为一组，对应的标记为true，则显示，否则不显示;
		avmLineDynamicSingleHandle.bDemoorNot[1] = true;
		avmLineDynamicSingleHandle.bDemoorNot[2] = false;
		avmLineDynamicSingleHandle.bDemoorNot[3] = false;
		}
		xcb_avmlib_singleView_Render(&avm_single_Handle, cameraSingle, &camera_state, texture_id);

		xcb_avm_line_dynamic_single_Update(&avmLineDynamicSingleHandle,
                                     &avm_single_Handle, &avm_car_Handle);
		xcb_avm_line_dynamic_single_Render(&avmLineDynamicSingleHandle);
		break;
	}
	case or_b:
	{
		avm_single_Handle.view = or_b;
		avmLineDynamicSingleHandle.line_dynamic_single->single.isResived=false;
		avmLineDynamicSingleHandle.line_dynamic_single->type=1;
		if(avmLineDynamicSingleHandle.line_dynamic_single->fAngle>0)
		{
		avmLineDynamicSingleHandle.bDemoorNot[0] = false; //每三根线为一组，对应的标记为true，则显示，否则不显示;
		avmLineDynamicSingleHandle.bDemoorNot[1] = true;
		avmLineDynamicSingleHandle.bDemoorNot[2] = false;
		avmLineDynamicSingleHandle.bDemoorNot[3] = false;
		}else
		{
		avmLineDynamicSingleHandle.bDemoorNot[0] = false; //每三根线为一组，对应的标记为true，则显示，否则不显示;
		avmLineDynamicSingleHandle.bDemoorNot[1] = false;
		avmLineDynamicSingleHandle.bDemoorNot[2] = true;
		avmLineDynamicSingleHandle.bDemoorNot[3] = false;
		}
		xcb_avmlib_singleView_Render(&avm_single_Handle, cameraSingle, &camera_state, texture_id);
		xcb_avm_line_dynamic_single_Update(&avmLineDynamicSingleHandle,
                                     &avm_single_Handle, &avm_car_Handle);
		xcb_avm_line_dynamic_single_Render(&avmLineDynamicSingleHandle);
		
		break;
	}
	case or_l:
	{
	avm_single_Handle.view = name;
	xcb_avmlib_singleView_Render(&avm_single_Handle, cameraSingle, &camera_state, texture_id);
	break;
	}
	case or_r:
	{
		avm_single_Handle.view = or_r;
		xcb_avmlib_singleView_Render(&avm_single_Handle, cameraSingle, &camera_state, texture_id);
		break;
	}
	case sd_f:
	{
		xcb_avmlib_sideView_Render(&avm_single_Handle, cameraSingle, &camera_state, texture_id,0);
	}
	case sd_b:
	{
		xcb_avmlib_sideView_Render(&avm_single_Handle, cameraSingle, &camera_state, texture_id,1);
	}
	default:
	{
		avm_single_Handle.view = or_f;
		xcb_avmlib_singleView_Render(&avm_single_Handle, cameraSingle, &camera_state, texture_id);

	}
	
	// case ud_f:
	// {
	// avm_single_Handle.view = ud_f;
	// avmLineDynamicSingleHandle.line_dynamic_single->single.isResived=true;
	// avmLineDynamicSingleHandle.line_dynamic_single->type=0;
	// 	if(avmLineDynamicSingleHandle.line_dynamic_single->fAngle>0)
	// 	{
	// 	avmLineDynamicSingleHandle.bDemoorNot[0] = false; //每三根线为一组，对应的标记为true，则显示，否则不显示;
	// 	avmLineDynamicSingleHandle.bDemoorNot[1] = true;
	// 	avmLineDynamicSingleHandle.bDemoorNot[2] = false;
	// 	avmLineDynamicSingleHandle.bDemoorNot[3] = false;
	// 	}else
	// 	{
	// 	avmLineDynamicSingleHandle.bDemoorNot[0] = true; //每三根线为一组，对应的标记为true，则显示，否则不显示;
	// 	avmLineDynamicSingleHandle.bDemoorNot[1] = false;
	// 	avmLineDynamicSingleHandle.bDemoorNot[2] = false;
	// 	avmLineDynamicSingleHandle.bDemoorNot[3] = false;
	// 	}
	// xcb_avm_line_dynamic_single_Update(&avmLineDynamicSingleHandle,
    //                                  &avm_single_Handle, &avm_car_Handle);
	// xcb_avm_line_dynamic_single_Render(&avmLineDynamicSingleHandle);
	// break;
	// }
	// case ud_b:
	// {
	// avm_single_Handle.view = ud_b;
	// avmLineDynamicSingleHandle.line_dynamic_single->single.isResived=true;
	// avmLineDynamicSingleHandle.line_dynamic_single->type=1;
	// if(avmLineDynamicSingleHandle.line_dynamic_single->fAngle>0)
	// 	{
	// 	avmLineDynamicSingleHandle.bDemoorNot[0] = false; //每三根线为一组，对应的标记为true，则显示，否则不显示;
	// 	avmLineDynamicSingleHandle.bDemoorNot[1] = false;
	// 	avmLineDynamicSingleHandle.bDemoorNot[2] = true;
	// 	avmLineDynamicSingleHandle.bDemoorNot[3] = false;
	// 	}else
	// 	{
	// 	avmLineDynamicSingleHandle.bDemoorNot[0] = false; //每三根线为一组，对应的标记为true，则显示，否则不显示;
	// 	avmLineDynamicSingleHandle.bDemoorNot[1] = true;
	// 	avmLineDynamicSingleHandle.bDemoorNot[2] = false;
	// 	avmLineDynamicSingleHandle.bDemoorNot[3] = false;
	// 	}
	// xcb_avm_line_dynamic_single_Update(&avmLineDynamicSingleHandle,
    //                                  &avm_single_Handle, &avm_car_Handle);
	// xcb_avm_line_dynamic_single_Render(&avmLineDynamicSingleHandle);
	// break;
	// }
	}
   
}

void set_single_2D_3D_vision_viewParam()//相机虚拟视角
{
  cameraParameter.win_x = 0;
  cameraParameter.win_y = 0;
  cameraParameter.win_width = WINDOW_2D_W;
  cameraParameter.win_height = WINDOW_2D_H;
  cameraParameter.fov = 85;
  cameraParameter.aspect_ratio = 3.0 / 4.8;
  cameraParameter.ex = 0;
  cameraParameter.ey = 0;
  cameraParameter.ez = 6000;//6000
  cameraParameter.tx = 0;
  cameraParameter.ty = 0;
  cameraParameter.tz = 0;
#if __stdlib__
  xcb_avmlib_2d_Set_ViewParameter(cameraParameter, 90);
#else
  xcb_avmlib_2d_Set_ViewParameter(cameraParameter);
#endif
  cameraParameter3D.win_x = WINDOW_2D_W;
  cameraParameter3D.win_y = 0;
  cameraParameter3D.win_width = WINDOW_W-WINDOW_2D_W;
  cameraParameter3D.win_height = WINDOW_H;
  cameraParameter3D.fov = 35.0;
  cameraParameter3D.aspect_ratio = 5.0/4.8;
  cameraParameter3D.ex = 0;
  cameraParameter3D.ey = 0;
  cameraParameter3D.ez = 3000;
  cameraParameter3D.tx = 0;
  cameraParameter3D.ty = 0;
  cameraParameter3D.tz = 150;

  xcb_avmlib_3d_Set_ViewParameter(cameraParameter3D);

  cameraSingle.win_x =WINDOW_2D_W;
  cameraSingle.win_y = 0;
  cameraSingle.win_width = WINDOW_W-WINDOW_2D_W;
  cameraSingle.win_height = WINDOW_H;
  cameraSingle.fov = 75.0;
  cameraSingle.aspect_ratio =5.0/4.8;
  cameraSingle.ex = 0;
  cameraSingle.ey = 0;
  cameraSingle.ez = 800;//480
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
	#ifdef USDMA
	avm_single_Handle.isDMA = true;
	#else
	avm_single_Handle.isDMA = false;
	#endif

	xcb_avmlib_single_Init(&avm_single_Handle); // 单视图初始化
	xcb_avmlib_single_Gen_VBO(NULL, "./gpu_data_default/SingleModel",
								&avm_single_Handle); //"./auth_data/SingleModel"    // 读取单视图 vbo 模型文件

	xcb_avmlib_sideView_Gen_VBO(NULL, "./gpu_data_default/SideViewModelF", &avm_single_Handle, 0); // 读取侧面视图 vbo 模型文件
	xcb_avmlib_sideView_Gen_VBO(NULL, "./gpu_data_default/SideViewModelB", &avm_single_Handle, 1);
	//xcb_avmlib_wideAngleView_Gen_VBO(NULL, "./gpu_data_default/BroadViewModel", &avm_single_Handle);
	
}

void change_car_skin()
{
	xcb_avmlib_UpdateMeterial((char *)"body", bodycolor[nBodyColorIndex]);
	nBodyColorIndex++;
	nBodyColorIndex = nBodyColorIndex % nMaxBodyColorNum;
	xcb_avmlib_car_SetReflectanceRatio(0.3);
}


void car_tyre_wheel_roll()
{
	static int itire = 0;
	avm_car_Handle.car_3d.wheel.roll_angle = itire * 12;
	avm_car_Handle.car_3d.wheel.swing_angle = 0;
	xcb_avmlib_carState_Update(&avm_car_Handle, true, false);
	itire++;
	itire %= 30;
}

///////////////2的俯视图lines///////////加载线使用的图片
void Bird_DynamicLine_init()
{
  xcb_avm_line_dynamic_bird_Init(&avmLineDynamicBirdHandle0); //注意：初始化之后可以将line_png.texture释放掉此处未释放，根据自己需求
  avmLineDynamicBirdHandle0.iCarLength = 5000;
  avmLineDynamicBirdHandle0.iLineDistance = 2000;
  avmLineDynamicBirdHandle0.iFrontWheelDis = 1200;
  avmLineDynamicBirdHandle0.iRearWheelDis = 1200;
  avmLineDynamicBirdHandle0.iWheelBase = 2800;
  avmLineDynamicBirdHandle0.bDemoorNot[0] = true; //每三根线为一组，对应的标记为true，则显示，否则不显示;
  avmLineDynamicBirdHandle0.bDemoorNot[1] = true;
  avmLineDynamicBirdHandle0.bDemoorNot[2] = true;
  avmLineDynamicBirdHandle0.bDemoorNot[3] = false;
  avmLineDynamicBirdHandle0.fratio[0] = 1.0; //一组三根线，每根线对应的透明度
  avmLineDynamicBirdHandle0.fratio[1] = 1.0;
  avmLineDynamicBirdHandle0.fratio[2] = 1.0;
  avmLineDynamicBirdHandle0.fratio[3] = 0;
  avmLineDynamicBirdHandle0.nSingleLineMode = 0;
  avmLineDynamicBirdHandle0.line_dynamic_bird->type = 0;
 
  avmLineDynamicBirdHandle0.line_dynamic_bird->fMaxAngle = 33.0;
  avmLineDynamicBirdHandle0.line_dynamic_bird->fAngle = 0; //-33.841061,33.549667
  avmLineDynamicBirdHandle0.line_dynamic_bird->pointnum = 60;
  avmLineDynamicBirdHandle0.line_dynamic_bird->thick = 100;
  avmLineDynamicBirdHandle0.line_dynamic_bird->iLineLength = 2500;
  avmLineDynamicBirdHandle0.line_dynamic_bird->bird.view = &cameraParameter;
  xcb_avmlib_Load_Png("./Dline/orbit1/line_single_dynamic.png", line_png);
  avmLineDynamicBirdHandle0.line_dynamic_bird->tex.width = line_png.width;
  avmLineDynamicBirdHandle0.line_dynamic_bird->tex.heigth = line_png.height;
  avmLineDynamicBirdHandle0.line_dynamic_bird->tex.channels = 4;
  avmLineDynamicBirdHandle0.line_dynamic_bird->tex.pixel = line_png.texture;
  xcb_avm_line_dynamic_bird_Gen_VBO(&avmLineDynamicBirdHandle0, &avm_single_Handle);

  xcb_avm_line_dynamic_bird_Init(&avmLineDynamicBirdHandle1); //注意：初始化之后可以将line_png.texture释放掉此处未释放，根据自己需求
  avmLineDynamicBirdHandle1.iCarLength = 5000;
  avmLineDynamicBirdHandle1.iLineDistance = 2000;
  avmLineDynamicBirdHandle1.iFrontWheelDis = 1200;
  avmLineDynamicBirdHandle1.iRearWheelDis = 1200;
  avmLineDynamicBirdHandle1.iWheelBase = 2800;
  avmLineDynamicBirdHandle1.bDemoorNot[0] = true; //每三根线为一组，对应的标记为true，则显示，否则不显示;
  avmLineDynamicBirdHandle1.bDemoorNot[1] = false;
  avmLineDynamicBirdHandle1.bDemoorNot[2] = false;
  avmLineDynamicBirdHandle1.bDemoorNot[3] = false;
  avmLineDynamicBirdHandle1.fratio[0] = 1; //一组三根线，每根线对应的透明度
  avmLineDynamicBirdHandle1.fratio[1] = 1;
  avmLineDynamicBirdHandle1.fratio[2] = 1;
  avmLineDynamicBirdHandle1.fratio[3] = 0;

  avmLineDynamicBirdHandle1.nSingleLineMode = 0;
  avmLineDynamicBirdHandle1.line_dynamic_bird->type = 0;
  avmLineDynamicBirdHandle1.line_dynamic_bird->fMaxAngle = 33.0;
  avmLineDynamicBirdHandle1.line_dynamic_bird->fAngle = -33.0; //-33.841061,33.549667
  avmLineDynamicBirdHandle1.line_dynamic_bird->pointnum = 60;
  avmLineDynamicBirdHandle1.line_dynamic_bird->thick = 100;
  avmLineDynamicBirdHandle1.line_dynamic_bird->iLineLength = 2500;
  avmLineDynamicBirdHandle1.line_dynamic_bird->bird.view = &cameraParameter;

  xcb_avmlib_Load_Png("./Dline/orbit1/line_single_dynamic1.png", line_png);
  avmLineDynamicBirdHandle1.line_dynamic_bird->tex.width = line_png.width;
  avmLineDynamicBirdHandle1.line_dynamic_bird->tex.heigth = line_png.height;
  avmLineDynamicBirdHandle1.line_dynamic_bird->tex.channels = 4;
  avmLineDynamicBirdHandle1.line_dynamic_bird->tex.pixel = line_png.texture;
  xcb_avm_line_dynamic_bird_Gen_VBO(&avmLineDynamicBirdHandle1, &avm_single_Handle);


  xcb_avm_line_dynamic_bird_Init(&avmLineDynamicBirdHandle2); //注意：初始化之后可以将line_png.texture释放掉此处未释放，根据自己需求
  avmLineDynamicBirdHandle2.iCarLength = 5000;
  avmLineDynamicBirdHandle2.iLineDistance = 2000;
  avmLineDynamicBirdHandle2.iFrontWheelDis = 1200;
  avmLineDynamicBirdHandle2.iRearWheelDis = 1200;
  avmLineDynamicBirdHandle2.iWheelBase = 2800;
  avmLineDynamicBirdHandle2.bDemoorNot[0] = true; //每三根线为一组，对应的标记为true，则显示，否则不显示;
  avmLineDynamicBirdHandle2.bDemoorNot[1] = false;
  avmLineDynamicBirdHandle2.bDemoorNot[2] = false;
  avmLineDynamicBirdHandle2.bDemoorNot[3] = false;
  avmLineDynamicBirdHandle2.fratio[0] = 1; //一组三根线，每根线对应的透明度
  avmLineDynamicBirdHandle2.fratio[1] = 1;
  avmLineDynamicBirdHandle2.fratio[2] = 1;
  avmLineDynamicBirdHandle2.fratio[3] = 0;

  avmLineDynamicBirdHandle2.nSingleLineMode = 0;
  avmLineDynamicBirdHandle2.line_dynamic_bird->type = 0;
  avmLineDynamicBirdHandle2.line_dynamic_bird->fMaxAngle = 33.0;
  avmLineDynamicBirdHandle2.line_dynamic_bird->fAngle = 33.0; //-33.841061,33.549667
  avmLineDynamicBirdHandle2.line_dynamic_bird->pointnum = 120;
  avmLineDynamicBirdHandle2.line_dynamic_bird->thick = 100;
  avmLineDynamicBirdHandle2.line_dynamic_bird->iLineLength = 2500;
  avmLineDynamicBirdHandle2.line_dynamic_bird->bird.view = &cameraParameter;

  xcb_avmlib_Load_Png("./Dline/orbit1/line_single_dynamic1.png", line_png);
  avmLineDynamicBirdHandle2.line_dynamic_bird->tex.width = line_png.width;
  avmLineDynamicBirdHandle2.line_dynamic_bird->tex.heigth = line_png.height;
  avmLineDynamicBirdHandle2.line_dynamic_bird->tex.channels = 4;
  avmLineDynamicBirdHandle2.line_dynamic_bird->tex.pixel = line_png.texture;

  xcb_avm_line_dynamic_bird_Gen_VBO(&avmLineDynamicBirdHandle2, &avm_single_Handle);
}

////////////////单视图lines///////////////////
void Single_DynamicLine_init()
{
	///////////////单视图动态轨迹lines///////////加载线使用的图片
	xcb_avm_line_dynamic_single_Init(&avmLineDynamicSingleHandle); //注意：初始化之后可以将line_png.texture释放掉此处未释放，根据自己需求
	avmLineDynamicSingleHandle.iCarLength = 5000;
	avmLineDynamicSingleHandle.iLineDistance = -2000;
	avmLineDynamicSingleHandle.iFrontWheelDis = 1000;
	avmLineDynamicSingleHandle.iRearWheelDis = 1000;
	avmLineDynamicSingleHandle.iWheelBase = 2800;
	avmLineDynamicSingleHandle.bDemoorNot[0] = false; //每三根线为一组，对应的标记为true，则显示，否则不显示;
	avmLineDynamicSingleHandle.bDemoorNot[1] = true;
	avmLineDynamicSingleHandle.bDemoorNot[2] = false;
	avmLineDynamicSingleHandle.bDemoorNot[3] = false;
	avmLineDynamicSingleHandle.fratio[0] = 1; //一组三根线，每根线对应的透明度
	avmLineDynamicSingleHandle.fratio[1] = 1;
	avmLineDynamicSingleHandle.fratio[2] = 1;
	avmLineDynamicSingleHandle.fratio[3] = 0;
	avmLineDynamicSingleHandle.nSingleLineMode = 0;
	avmLineDynamicSingleHandle.line_dynamic_single->single.isResived = false;
	avmLineDynamicSingleHandle.line_dynamic_single->type = 0;
	avmLineDynamicSingleHandle.line_dynamic_single->fMaxAngle = 33.0;
	avmLineDynamicSingleHandle.line_dynamic_single->fAngle = 0; //-33.841061,33.549667
	avmLineDynamicSingleHandle.line_dynamic_single->pointnum = 120;
	avmLineDynamicSingleHandle.line_dynamic_single->thick = 2000;
	avmLineDynamicSingleHandle.line_dynamic_single->iLineLength = 2500;

	avmLineDynamicSingleHandle.line_dynamic_single->single.view = &cameraSingle;

	xcb_avmlib_Load_Png("./Dline/orbit3/line_single_dynamic.png", line_png);
	printf("db------------------------------4\n");
	avmLineDynamicSingleHandle.line_dynamic_single->tex.width = line_png.width;
	avmLineDynamicSingleHandle.line_dynamic_single->tex.heigth = line_png.height;
	avmLineDynamicSingleHandle.line_dynamic_single->tex.channels = 4;
	avmLineDynamicSingleHandle.line_dynamic_single->tex.pixel = line_png.texture;

  	xcb_avm_line_dynamic_single_Gen_VBO(&avmLineDynamicSingleHandle,
                                      &avm_single_Handle);
  /// end single
}

void update_line_angle(float tyreAngle)//输入挡位以及轮胎角度（单位度）
{        
		avmLineDynamicSingleHandle.line_dynamic_single->fAngle = tyreAngle;
		avmLineDynamicBirdHandle0.line_dynamic_bird->fAngle =tyreAngle;
        if (fabs(avmLineDynamicBirdHandle0.line_dynamic_bird->fAngle) < 0.011)
        {
          //avmLineDynamicBirdHandle0.line_dynamic_bird->fAngle = 0;
          avmLineDynamicBirdHandle0.bDemoorNot[2] = false;
        }
        else
        {

          avmLineDynamicBirdHandle0.bDemoorNot[2] = true;
        }
		xcb_avm_line_dynamic_bird_Update(&avmLineDynamicBirdHandle0, &avm_single_Handle, &avm_car_Handle);
		xcb_avm_line_dynamic_bird_Update(&avmLineDynamicBirdHandle1, &avm_single_Handle, &avm_car_Handle);
		xcb_avm_line_dynamic_bird_Update(&avmLineDynamicBirdHandle2, &avm_single_Handle, &avm_car_Handle);

}
//===============================ground view===============================
void trans_ground_view_init()
{
	avm_transchassis_car_info avm_trancar_blendinginfor;

	avm_trancar_blendinginfor.fCar_length = 5000;
	avm_trancar_blendinginfor.fCar_width = 2050;
	avm_trancar_blendinginfor.fCar_AxisLenth = 2670;
	avm_trancar_blendinginfor.fCar_backaxis2back = 1190;
	avm_trancar_blendinginfor.fCar_lrtyredist = 1540;
	avm_trancar_blendinginfor.length_gravity_backaxis = 1335;
	avm_trancar_blendinginfor.bind_area[0] = 0-2100/2;//avm_car_Handle.blind_2d->blind_area_cx - avm_car_Handle.blind_2d->blind_area_width / 2.0;
	avm_trancar_blendinginfor.bind_area[1] = 0+5500/2;//avm_car_Handle.blind_2d->blind_area_cy + avm_car_Handle.blind_2d->blind_area_length / 2.0;
	avm_trancar_blendinginfor.bind_area[2] = 0+2100/2;//avm_car_Handle.blind_2d->blind_area_cx + avm_car_Handle.blind_2d->blind_area_width / 2.0;
	avm_trancar_blendinginfor.bind_area[3] = 0+5500/2;//avm_car_Handle.blind_2d->blind_area_cy + avm_car_Handle.blind_2d->blind_area_length / 2.0;
	avm_trancar_blendinginfor.bind_area[4] = 0+2100/2;//avm_car_Handle.blind_2d->blind_area_cx + avm_car_Handle.blind_2d->blind_area_width / 2.0;
	avm_trancar_blendinginfor.bind_area[5] = 0-5500/2;//avm_car_Handle.blind_2d->blind_area_cy - avm_car_Handle.blind_2d->blind_area_length / 2.0;
	avm_trancar_blendinginfor.bind_area[6] = 0-2100/2;//avm_car_Handle.blind_2d->blind_area_cx - avm_car_Handle.blind_2d->blind_area_width / 2.0;
	avm_trancar_blendinginfor.bind_area[7] = 0-5500/2;//avm_car_Handle.blind_2d->blind_area_cy - avm_car_Handle.blind_2d->blind_area_length / 2.0;

	avm_trancar_blendinginfor.iou_area[0] = 0-2100/20;//avm_car_Handle.blind_2d->blind_area_cx - avm_car_Handle.blind_2d->blind_area_width / 2.0;
	avm_trancar_blendinginfor.iou_area[1] = 0+5500/20;//avm_car_Handle.blind_2d->blind_area_cy + avm_car_Handle.blind_2d->blind_area_length / 8.;
	avm_trancar_blendinginfor.iou_area[2] = 0+2100/20;//avm_car_Handle.blind_2d->blind_area_cx + avm_car_Handle.blind_2d->blind_area_width / 2.0;
	avm_trancar_blendinginfor.iou_area[3] = 0+5500/20;//avm_car_Handle.blind_2d->blind_area_cy + avm_car_Handle.blind_2d->blind_area_length / 8.;
	avm_trancar_blendinginfor.iou_area[4] = 0+2100/20;//avm_car_Handle.blind_2d->blind_area_cx + avm_car_Handle.blind_2d->blind_area_width / 2.0;
	avm_trancar_blendinginfor.iou_area[5] = 0-5500/20;//avm_car_Handle.blind_2d->blind_area_cy - avm_car_Handle.blind_2d->blind_area_length / 8.;
	avm_trancar_blendinginfor.iou_area[6] = 0-2100/20;//avm_car_Handle.blind_2d->blind_area_cx - avm_car_Handle.blind_2d->blind_area_width / 2.0;
	avm_trancar_blendinginfor.iou_area[7] = 0-5500/20;//avm_car_Handle.blind_2d->blind_area_cy - avm_car_Handle.blind_2d->blind_area_length / 8.;

	avm_fbo_transchassis.nPatch_Bin_X = 47;
	avm_fbo_transchassis.nPatch_Bin_Y = 45;
	avm_fbo_transchassis.nChangeIndexornot = 0;
	avm_fbo_transchassis.nFirstRender = 1;
	avm_fbo_transchassis.patchsize_height = 10800; /////////////////这里的设置需要同步配置文件的a、b
	avm_fbo_transchassis.patchsize_width = 8000;
	avm_fbo_transchassis.window_width = 560;
	avm_fbo_transchassis.window_height = 480;
	avm_fbo_transchassis.ISDMA=1;
	avm_fbo_transchassis.fBlindAreaWidth = 2300;
	avm_fbo_transchassis.fBlindAreaHeight = 5800;
	avm_fbo_transchassis.fBlindcx = 0;
	avm_fbo_transchassis.fBlindcy = -100;
	avm_fbo_transchassis.blending_bin_x = 2;
	avm_fbo_transchassis.blending_bin_y = 3;
	xcb_avm_fbo_transchassis_Init(&avm_fbo_transchassis, &avm_stitching_Handle);

	int npoint = avm_fbo_transchassis.fbo_transchassis_patch->vertex_number;
	printf("init..blendingX%d blendingY=%d......................\n", avm_fbo_transchassis.blending_bin_x, avm_fbo_transchassis.blending_bin_y);
	printf("%f %f %f %f", avm_fbo_transchassis.fBlindAreaWidth, avm_fbo_transchassis.fBlindAreaHeight,
			avm_fbo_transchassis.patchsize_width, avm_fbo_transchassis.patchsize_height);

    ground_view_arr = xcb_avm_transchassis_coordintatetransfer_init(&avm_fbo_transchassis, &avm_trancar_blendinginfor);

	car_move_info.ftyrev[0] =5.0;
	car_move_info.ftyrev[1] =5.0;
	car_move_info.ftyrev[2] =5.0;
	car_move_info.ftyrev[3] =5.0;
	car_move_info.tyretheta = 0;
    car_move_info.timeinteval = (long long)50;
}

//===============================ADAS_MOD+BSD+FcW==========================

void *Mod_process(void *arg) 
{
	
	bool cameraON[4] = {1,1,1,1 };
	const char *auth_file = "./gpu_data_default/avm_license";
	char calibOutPath[500], calibOutPath_default[500];

	sprintf(calibOutPath, NULL);
	sprintf(calibOutPath_default, "./gpu_data_default/mod_params");

	constexpr int rows  = 720;
	constexpr int cols  = 1280;
	int dist_horizontal = 300;
	int dist_vertical   = 300;
	float fAngle        = 0;
	int nOutputNum      = 0;
	sf_xcb_mod_car_gear_state_t emGearStatus = Drive_gear;
	float fSpeed=0;
	sf_xcb_mod_output_s *mod_output=0;
	void *pMOD = sf_xcb_mod_init(dist_horizontal, dist_vertical, cols, rows,calibOutPath, calibOutPath_default,auth_file);
	
	
	while (1)
	{
		fSpeed = (float)0;
		{
			
			
//			memcpy(ADASIMG[0],NV21IMG[0], NV21SIZE);
//			memcpy(ADASIMG[1],NV21IMG[1], NV21SIZE);
//			memcpy(ADASIMG[2],NV21IMG[2], NV21SIZE);
//			memcpy(ADASIMG[3],NV21IMG[3], NV21SIZE);
			
	        sf_xcb_mod_process(pMOD, NV21IMG[0],NV21IMG[1],NV21IMG[2],NV21IMG[3],fSpeed,Parking_gear,fAngle, &mod_output);
			
			if (mod_output)
			{
				if(mod_output[0].nObjNum > 0)
				{
					//printf("MOD CAR Back: %d objects are detected!\n", mod_output[0].nObjNum);
					// //MOD 后方报警
					which_side_mod_alarm[SF_XCB_MOD_Back] = true;
				}
				else
				{
					which_side_mod_alarm[SF_XCB_MOD_Back] = false;
				}

				if (mod_output[1].nObjNum > 0)
				{
					//printf("MOD CAR L: %d objects are detected!\n", mod_output[1].nObjNum);
					which_side_mod_alarm[SF_XCB_MOD_Left] = true;;
				}
				else
				{
					which_side_mod_alarm[SF_XCB_MOD_Left]= false;
				}

				if (mod_output[2].nObjNum > 0)
				{
					//printf("MOD CAR F: %d objects are detected!\n", mod_output[2].nObjNum);
					which_side_mod_alarm[SF_XCB_MOD_Front] = true;
				}
				else
				{
					which_side_mod_alarm[SF_XCB_MOD_Front] = false;
				}

				if (mod_output[3].nObjNum > 0)
				{
					which_side_mod_alarm[SF_XCB_MOD_Right]= true;
					//printf("MOD CAR RIGHT: %d objects are detected!\n", mod_output[3].nObjNum);
					// //MOD 右边报警
				}
				else
				{
					which_side_mod_alarm[SF_XCB_MOD_Right] = false;
				}

			}
			
			if(which_side_mod_alarm[SF_XCB_MOD_Back]||which_side_mod_alarm[SF_XCB_MOD_Left]
			||which_side_mod_alarm[SF_XCB_MOD_Front]||which_side_mod_alarm[SF_XCB_MOD_Right]
			)
			{		
				MOD_allarm=true;
			}else
			{MOD_allarm=false;}
			
		}
		
	}

	
	sf_xcb_mod_release(&pMOD);

	pthread_exit(NULL);
}


/**************************************************************************************/
void *Bsd_process(void *arg)
{
	constexpr int rows 	= 720;
	constexpr int cols 	= 1280;
	int dist_horizontal 	= 300;
	int dist_vertical 	= 300;
	float fActSpeed 	= 30.0f;
	const char *auth_file 	= "./gpu_data_default/avm_license";
	int idx = 0;

	char calibOutPath[500], calibOutPath_default[500];

	sprintf(calibOutPath, NULL);
	sprintf(calibOutPath_default, "./gpu_data_default/bsd_params");

	bool bbsdl  = false;
	bool bbsdr  = false;
	bool bbsdlr = false;
	sf_xcb_bsd_event_t *event=0;
	void *pBSD = sf_xcb_bsd_init(fActSpeed, dist_horizontal, dist_vertical, cols,rows, calibOutPath, calibOutPath_default,auth_file);
	
	while (1)
	{
		
	
		float fCurSpeedBsd  = (float)40;//获取实际速度


		if((fCurSpeedBsd > 30) && (fCurSpeedBsd < 120))//if((fCurSpeed > 30) && (fCurSpeed < 120))
		{
	
//			memcpy(ADASIMG[0],NV21IMG[0], NV21SIZE);

      		sf_xcb_bsd_process(pBSD, NV21IMG[0], fCurSpeedBsd, &event);
			sf_xcb_bsd_get_detect_objects(pBSD,(sf_xcb_bsd_output_t **)&detect_obj);
			sf_xcb_bsd_get_objects(pBSD,(sf_xcb_bsd_output_t **)&allarm_obj);
			// if(detect_obj->nRect>0)
			// {
				BSD_allarm=true;
			// printf	printf("bsd is allarm:%d\n",BSD_allarm);
			
			// }
			// else
			// {
			// 	BSD_allarm=false;
			// }
			if (event)
			{
				if (*event == sf_xcb_bsd_event_t::SF_XCB_BSD_Left_Warning)
				{
					printf("|******* Warning from Left of car!**********|\n");
				
					BSD_allarm=true;
				}
				else if (*event == sf_xcb_bsd_event_t::SF_XCB_BSD_Right_Warning)
				{
					printf("|******* Warning from Right of car!**********|\n");
				
					BSD_allarm=true;
				}
				else if (*event == sf_xcb_bsd_event_t::SF_XCB_BSD_Left_Right_Warning)
				{
					printf("|******* Warning from both Sides of car!**********|\n");
					BSD_allarm=true;
				}
				else 
				{
				    //printf("bsd ============run\n");
					BSD_allarm=false;
				}
			}
		}
		
	}
	sf_xcb_bsd_release(&pBSD);

	pthread_exit(NULL);
}

/**************************************************************************************/
/**************************************************************************************/
void  *fcw_process(void *arg)
{
	constexpr int rows = 720;
	constexpr int cols = 1280;
	int dist_horizontal = 300;
	int dist_vertical = 300;

	const char *auth_file = "./gpu_data_default/avm_license";
	sf_xcb_fcw_output_t *fcwoutput=0;
	char pCalibOutPath[500], pDefaultCalibOutPath[500];   

	int light_init_flag = 0;
	void *pLightavg = NULL;
	float fratio[4] = {1.0, 1.0, 1.0, 1.0};
	char param_path[200];
	char default_param_path[200];

	sprintf(pCalibOutPath, "./gpu_data_default/CalibExParam");
	sprintf(pDefaultCalibOutPath, "./gpu_data_default/CalibExParam");

	void *pfcw = sf_xcb_fcw_init(dist_horizontal, dist_vertical,cols, rows, pCalibOutPath, pDefaultCalibOutPath,auth_file); //fcw init 
	while (1)
	{
		float fCurSpeedFcw  = (float)40;//获取实时速度
		if(fCurSpeedFcw > 30)
		{	
			
//			memcpy(ADASIMG[2],NV21IMG[2], NV21SIZE);
			
			sf_xcb_fcw_process(pfcw, NV21IMG[2], fCurSpeedFcw); //传入前摄像头数据（unsigned char* 1280*720*3/2）和当前车速 m/s
			sf_xcb_fcw_get_detect_obj(pfcw, &fcwoutput);
			
			//printf("FCW ============run\n");
			if (fcwoutput)
			{
			
				for (int i = 0; i < fcwoutput->nObjNum; i++)
				{
					if (fcwoutput->obj[i].distance < 40 )//前方车辆运动速度低于本车速度且距离小于15m 
					{
						
						//printf("FCW warning is ============true\n");
						//fcw_detected = true;
						FCW_allarm=true;
					}
					else
					{
						FCW_allarm=false;
						//printf("FCW warning is ============false\n");
						//fcw_detected = false;
					}
				}
				
			}
			else
			{
			   FCW_allarm = false;
			}
		}

		
	}

	sf_xcb_fcw_release(&pfcw);

	pthread_exit(NULL);
}


//===============================mod_ui_porcess=======================================
void adas_UI_init()
{
	//logo
	// logoUIHandle.win_x = (WINDOW_W-600)/2;
	// logoUIHandle.win_y = WINDOW_H/2;
	// logoUIHandle.width=600; //图片的宽高 不然可能会花图
	// logoUIHandle.height=40;
	// xcb_avmlib_Load_Png("./Dline/nosale.png", logo_png_texture);

	// xcb_avmlib_Draw_Png_Init(&logoUIHandle,&logo_png_texture,1);
	// xcb_avmlib_Update_Png(logo_png_texture,&logoUIHandle);


	//MOD ui handle;
	modUIHandle.win_x = 0;
	modUIHandle.win_y = 0;
	modUIHandle.width=38; //图片的宽高 不然可能会花图
	modUIHandle.height=34;
	xcb_avmlib_Load_Png("./Dline/mod_warning.png", mod_png_texture);

	xcb_avmlib_Draw_Png_Init(&modUIHandle,&mod_png_texture,1);
	xcb_avmlib_Update_Png(mod_png_texture,&modUIHandle);
	printf("load png-----[x%d y%d w%d h%d]\n",modUIHandle.win_x,modUIHandle.win_y,modUIHandle.width,modUIHandle.height);
	//bsd ui handle;
	
	xcb_avm_adas_Init(&bsd_ui_handle);
	printf("bsd ui handle init\n");
	//fcw ui handle;
}

void mod_ui_porcess()
{
	//draw allarm Icon;
	if(which_side_mod_alarm[SF_XCB_MOD_Back])
	{
		//printf("-----------------------mod-----------Back-----ui\n");
		modUIHandle.win_x=WINDOW_2D_W*0.5-modUIHandle.width*0.5;
		modUIHandle.win_y=50;
		///printf("[x%d y%d w%d h%d]\n",modUIHandle.win_x,modUIHandle.win_y,modUIHandle.width,modUIHandle.height);
		///xcb_avmlib_Update_Png(mod_png_texture,&modUIHandle);
		xcb_avmlib_Draw_Png(&modUIHandle);
	}
	if(which_side_mod_alarm[SF_XCB_MOD_Left])
	{
		modUIHandle.win_x=50-mod_png_texture.width*0.5;
		modUIHandle.win_y=WINDOW_2D_H*0.5;
		//xcb_avmlib_Update_Png(mod_png_texture,&modUIHandle);
		xcb_avmlib_Draw_Png(&modUIHandle);
		//printf("-----------------------mod-----------Left-----ui\n");	
		//printf("[x%d y%d w%d h%d]\n",modUIHandle.win_x,modUIHandle.win_y,modUIHandle.width,modUIHandle.height);


	}
	if(which_side_mod_alarm[SF_XCB_MOD_Front])
	{
		modUIHandle.win_x=WINDOW_2D_W*0.5-mod_png_texture.width*0.5;
		modUIHandle.win_y=WINDOW_2D_H-50;
		//xcb_avmlib_Update_Png(mod_png_texture,&modUIHandle);
		xcb_avmlib_Draw_Png(&modUIHandle);
		//printf("-----------------------mod-----------Front-----ui\n");
		//printf("[x%d y%d w%d h%d]\n",modUIHandle.win_x,modUIHandle.win_y,modUIHandle.width,modUIHandle.height);

	}
	if(which_side_mod_alarm[SF_XCB_MOD_Right])
	{
		modUIHandle.win_x=WINDOW_2D_W-50-mod_png_texture.width*0.5;
		modUIHandle.win_y=WINDOW_2D_H*0.5;
		//xcb_avmlib_Update_Png(mod_png_texture,&modUIHandle);
		xcb_avmlib_Draw_Png(&modUIHandle);
		///printf("-----------------------mod-----------Right-----ui\n");
		//printf("[x%d y%d w%d h%d]\n",modUIHandle.win_x,modUIHandle.win_y,modUIHandle.width,modUIHandle.height);

	}
     
}
//===============================bsd_ui_process=======================================
// void bsd_ui_process()
// {
// 	//draw obj box 
// 	int points_num=4;
// 	float points[8];
// 	int lux=0,luy=0,luz=0;
// 	int rux=0,ruy=0,ruz=0;
// 	int rdx=0,rdy=0,rdz=0;
// 	int ldx=0,ldy=0,ldz=0;
// 	printf("test5-------------draw detect obj num=%d \n",detect_obj->nRect);
// 	position=(bsd_position_t*)detect_obj->positions;
// 	for(int i=0;i<detect_obj->nRect;i++)
// 	{		
// 		points[0]=lux= CAMERA_WIDTH-position[i].x*2;
// 		points[1]=luy=-position[i].y*2 + CAMERA_HEIGHT;
// 		//points[2]=luz=0.1;
// 		points[2]=rux= CAMERA_WIDTH - (position[i].x+position[i].w)*2;
// 		points[3]=ruy=-position[i].y*2 + CAMERA_HEIGHT;
// 		//points[5]=ruz=0.1;
		
// 		points[4]=rdx=CAMERA_WIDTH-(position[i].x+position[i].w)*2;
// 		points[5]=rdy=-(position[i].y+position[i].h)*2 + CAMERA_HEIGHT;
// 		//points[8]=rdz=0.1;
		
// 		points[6]=ldx= CAMERA_WIDTH-position[i].x*2;
// 		points[7]=ldy=-(position[i].y+position[i].h)*2 + CAMERA_HEIGHT;
// 		//points[11]=ldz=0.1;

// 		printf("lup2[x%d y%d] rup[x%d y%d] rdp[x%d y%d] ldp[x%d y%d]\n",lux,luy,rux,ruy,rdx,rdy,ldx,ldy);	
// 		xcb_avm_adas_Update(&bsd_ui_handle, 4,points, 0);
// 		xcb_avm_adas_Render(&bsd_ui_handle, &cameraSingle,rect_color, 0);
// 	}
	
// }
//===============================fcw_ui_process=======================================
void fcw_ui_process()
{
	//draw obj box
}

void read_img_test()
{
	FILE *fp;
    #ifdef USDMA
    #else
	fp = fopen("./gpu_data_default/NV61_B", "rb");
	fread(pImg[0], sizeof(unsigned char), 1843200, fp);
	fclose(fp);
	fp = fopen("./gpu_data_default/NV61_L", "rb");
	fread(pImg[1], sizeof(unsigned char), 1843200, fp);
	fclose(fp);
	fp = fopen("./gpu_data_default/NV61_F", "rb");
	fread(pImg[2], sizeof(unsigned char), 1843200, fp);
	fclose(fp);
	fp = fopen("./gpu_data_default/NV61_R", "rb");
	fread(pImg[3], sizeof(unsigned char), 1843200, fp);
	fclose(fp);
	#endif
}

//+++++++++++++++++++++++++++++++++++ RECODER_TEST +++++++++++++++++++++++++++++++++//
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

//+++++++++++++++++++++++++++++++++++ RECODER_TEST +++++++++++++++++++++++++++++++++//

void *AVM_Camera(void *arg)//全景线程入口
{
	system("fbinit");
	xcb_avmlib_Window_Init(WINDOW_W, WINDOW_H, egl_display, egl_surface);//显示窗口初始化 屏幕大小1280*480
#ifdef USDMA
	xcb_avmlib_Gen_TextureBuffer_DMA(8, texture_id);
	avm_stitching_Handle.isDMA = true;
	printf("Thread DMA!\r\n");
#else
	xcb_avmlib_Gen_TextureBuffer(8, texture_id);
	printf("Thread not DMA!\r\n");
#endif
    XCB_TIME_START("avm init time")
	load_2d_3d_bowModle();//加载2D3D 拼接模型
	load_single_view_model();
	set_single_2D_3D_vision_viewParam();//设置2D 3D虚拟相机位置
	open3d_rotate_trail_data();//读取3D旋转轨迹
	car_model_init();
	//#ANCHOR //开始循环
	printf("init line start\n");
	Bird_DynamicLine_init();
	Single_DynamicLine_init();
	printf("finish init line\n");
//	trans_ground_view_init();
	printf("finish groundview\n");
	adas_UI_init();
	printf("finish UI init\n");
	XCB_TIME_END
	//===========
	float angleuseforline_debug=-33;
	int idx_view=1;
	printf("use DMA data\n");
//	read_img_test();
	pthread_cond_wait(&cond[0], &mutex[0]);
	pthread_cond_wait(&cond[1], &mutex[1]);
	pthread_cond_wait(&cond[2], &mutex[2]);
	pthread_cond_wait(&cond[3], &mutex[3]);	
#if RECODER_TEST	
	sem_post(&record_sem);
#endif
	while(1)
	{

		if(avm_run == 0)
		{
			printf("avm run pause\n");
			usleep(1000*1000);
			continue;
		}
		
		KeyboardInputProcess(Keystate);
#ifdef USDMA

      for (int i = 0; i < 4; i++)
      {
        xcb_avmlib_Update_TextureBuffer_DMA(elg_Image[i], texture_id, i);
      }	
	
#else
      xcb_avmlib_Update_TextureBuffer(pImg[0], pImg[1], pImg[2], pImg[3],
                                      texture_id);
#endif
		xcb_avmlib_Clean_Screen(0.1, 0.1, 0.1, 0.5);//清屏
    
		camera_state.camera_state[0] = true;
		camera_state.camera_state[1] = true;
		camera_state.camera_state[2] = true;
		camera_state.camera_state[3] = true;
	// 	xcb_avm_transchassis_coordintatetransfer_update(ground_view_arr, &car_move_info, &avm_fbo_transchassis);

    //   xcb_avm_fbo_transchassis_Update(&avm_fbo_transchassis);
        ///////////////////////////当运行距离超过一定距离后重置///////////////////
		if (NTT >= ntt)
		{
			NTT = 0;
			changeflag++;
			change_car_skin(); //每旋转360 换肤
		}
		//=================================透明底盘=========================================
        // pTexResult = xcb_avm_fbo_transchassis_Render(&avm_fbo_transchassis,
        //                                                     &avm_stitching_Handle, cameraParameter,
        //                                                     texture_id);

		
		if(angleuseforline_debug>33)
		{
			angleuseforline_debug=-33;
		}
		//printf("angle=%f\n",angleuseforline_debug);
		update_line_angle(angleuseforline_debug);
		
		if(changeflag%2==0)
		{
			cameraParameter3D.ex = p[NTT * 2];
			cameraParameter3D.ey = p[NTT * 2 + 1];
			car_tyre_wheel_roll();
			xcb_avmlib_3d_Set_ViewParameter(cameraParameter3D);
			Render_3D(use_ground_view);//3D拼接渲染
		}
		else
		{
			render_single((View_name)idx_view);
		}
		
//================================唤醒=================================

		// if(MOD_allarm)
		// {
		// 	mod_ui_porcess();
		// }		
		
//=====================================================================
		NTT++;

		angleuseforline_debug+=0.3;

		if(iKey==1)
		{
			use_ground_view=!use_ground_view;
		}
		if(iKey==2)
		{
			idx_view++;
		}
		if(iKey==3)
		{
			idx_view--;
		}
			
		// if(changeflag>13)
		// {
		// avm_single_Handle.view = or_b;
		// xcb_avmlib_singleView_Render(&avm_single_Handle, cameraSingle, &camera_state, texture_id);
		// //bsd_ui_process();
		// xcb_avm_adas_renderRectGroup(&bsd_ui_handle,&cameraSingle,detect_obj,or_b,rect_color,CAMERA_WIDTH,CAMERA_HEIGHT);
		// }
		iKey=0;
		Render_2D(use_ground_view);//渲染2D拼接画面

		xcb_avmlib_swapbuffer(egl_display, egl_surface);//渲染到屏幕
	}
#if 1
	pthread_mutex_unlock(&mutex[3]);
	pthread_mutex_unlock(&mutex[2]);
	pthread_mutex_unlock(&mutex[1]);
	pthread_mutex_unlock(&mutex[0]);
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
	
	struct buffer *buffers;
	
	//memset(buffers,0,sizeof(struct buffer));
	
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

	camera1 = 1;
	int ret;
	int i, j = 0;
	int device_id = 0;

	//pthread_mutex_lock(&mutex[0]);
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
		//pthread_mutex_unlock(&mutex[0]);
		return (void *)-1;
	} else
		printf("VIDIOC_STREAMON ok\n");
	
	//pthread_mutex_unlock(&mutex[0]);
	
	while (camera_run) {
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
//		memcpy(NV21IMG[0], gDispTab0[buf.index].vir, NV21SIZE); //B
#endif
		if(record_start){
			memcpy((void *)recordMem[device_id].vir,(void *)buffers0[buf.index].start[0],gDispTab0[buf.index].len);
			flushCache(cdx_memtype, &recordMem[device_id], NULL);
			v4l2_buf[device_id].width = 1280;
			v4l2_buf[device_id].height = 720;
			v4l2_buf[device_id].addrPhyY = recordMem[device_id].phy;
			v4l2_buf[device_id].addrPhyC = recordMem[device_id].phy + v4l2_buf[device_id].width*v4l2_buf[device_id].height;
			v4l2_buf[device_id].addrVirY = recordMem[device_id].vir;
			v4l2_buf[device_id].addrVirC = recordMem[device_id].vir + v4l2_buf[device_id].width*v4l2_buf[device_id].height;
			v4l2_buf[device_id].dmafd = recordMem[device_id].ion_buffer.fd_data.aw_fd;
			v4l2_buf[device_id].timeStamp =  (int64_t) systemTime();	
			pRecordCamera[device_id]->dataCallbackTimestamp(v4l2_buf[device_id].timeStamp,CAMERA_MSG_VIDEO_FRAME, 
										(char *)&v4l2_buf[device_id], NULL);
		}
		if (-1 == ioctl(fd[device_id], VIDIOC_QBUF, &buf)) {
			printf("VIDIOC_QBUF buf.index %d failed\n", buf.index);
			free(buf.m.planes);
			return (void *)-1;
		}

		free(buf.m.planes);
	}
	disp_disable();
	usleep(20*1000);

	if (-1 == ioctl(fd[device_id], VIDIOC_STREAMOFF, &type)) {
		printf("VIDIOC_STREAMOFF failed\n");
		return (void *)-1;
	} else
		printf("VIDIOC_STREAMOFF ok\n");

	if (-1 == free_frame_buffers(device_id))
		return (void *)-1;

	close(fd[device_id]); //close camera

	camera1=2;
	printf("camera0 close\n");
	
	pthread_exit(NULL);
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
	camera2 =1;
	//pthread_mutex_lock(&mutex[1]);
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
		//pthread_mutex_unlock(&mutex[1]);
		return (void *)-1;
	} else
		printf("VIDIOC_STREAMON ok\n");
	//pthread_mutex_unlock(&mutex[1]);
		
	while (camera_run) {
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
//		memcpy(NV21IMG[1], gDispTab1[buf.index].vir, NV21SIZE); //l

#endif
		if(record_start){
			memcpy((void *)recordMem[device_id].vir,(void *)buffers1[buf.index].start[0],gDispTab1[buf.index].len);
			flushCache(cdx_memtype, &recordMem[device_id], NULL);
			v4l2_buf[device_id].width = 1280;
			v4l2_buf[device_id].height = 720;
			v4l2_buf[device_id].addrPhyY = recordMem[device_id].phy;
			v4l2_buf[device_id].addrPhyC = recordMem[device_id].phy + v4l2_buf[device_id].width*v4l2_buf[device_id].height;
			v4l2_buf[device_id].addrVirY = recordMem[device_id].vir;
			v4l2_buf[device_id].addrVirC = recordMem[device_id].vir + v4l2_buf[device_id].width*v4l2_buf[device_id].height;
			v4l2_buf[device_id].dmafd = recordMem[device_id].ion_buffer.fd_data.aw_fd;
			v4l2_buf[device_id].timeStamp =  (int64_t) systemTime();	
			pRecordCamera[device_id]->dataCallbackTimestamp(v4l2_buf[device_id].timeStamp,CAMERA_MSG_VIDEO_FRAME, 
										(char *)&v4l2_buf[device_id], NULL);
		}

		if (-1 == ioctl(fd[device_id], VIDIOC_QBUF, &buf)) {
			printf("VIDIOC_QBUF buf.index %d failed\n", buf.index);
			free(buf.m.planes);
			return (void *)-1;
		}

		free(buf.m.planes);
	}
	disp_disable();
	usleep(20*1000);

	if (-1 == ioctl(fd[device_id], VIDIOC_STREAMOFF, &type)) {
		printf("VIDIOC_STREAMOFF failed\n");
		return (void *)-1;
	} else
		printf("VIDIOC_STREAMOFF ok\n");

	if (-1 == free_frame_buffers(device_id))
		return (void *)-1;
	
	close(fd[device_id]); //close camera
	camera2 = 2;
	printf("camera1 close\n");
	
	pthread_exit(NULL);
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
	camera3=1;
	//pthread_mutex_lock(&mutex[2]);
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
		//pthread_mutex_unlock(&mutex[2]);
		return (void *)-1;
	} else
		printf("VIDIOC_STREAMON ok\n");
	//pthread_mutex_unlock(&mutex[2]);
		
	while (camera_run) {
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
//		memcpy(NV21IMG[2], gDispTab2[buf.index].vir, NV21SIZE); //l

#endif
		if(record_start){
			memcpy((void *)recordMem[device_id].vir,(void *)buffers2[buf.index].start[0],gDispTab2[buf.index].len);
			flushCache(cdx_memtype, &recordMem[device_id], NULL);
			v4l2_buf[device_id].width = 1280;
			v4l2_buf[device_id].height = 720;
			v4l2_buf[device_id].addrPhyY = recordMem[device_id].phy;
			v4l2_buf[device_id].addrPhyC = recordMem[device_id].phy + v4l2_buf[device_id].width*v4l2_buf[device_id].height;
			v4l2_buf[device_id].addrVirY = recordMem[device_id].vir;
			v4l2_buf[device_id].addrVirC = recordMem[device_id].vir + v4l2_buf[device_id].width*v4l2_buf[device_id].height;
			v4l2_buf[device_id].dmafd = recordMem[device_id].ion_buffer.fd_data.aw_fd;
			v4l2_buf[device_id].timeStamp =  (int64_t) systemTime();	
			pRecordCamera[device_id]->dataCallbackTimestamp(v4l2_buf[device_id].timeStamp,CAMERA_MSG_VIDEO_FRAME, 
										(char *)&v4l2_buf[device_id], NULL);
		}

		if (-1 == ioctl(fd[device_id], VIDIOC_QBUF, &buf)) {
			printf("VIDIOC_QBUF buf.index %d failed\n", buf.index);
			free(buf.m.planes);
			return (void *)-1;
		}

		free(buf.m.planes);
	}
	disp_disable();
	usleep(20*1000);

	if (-1 == ioctl(fd[device_id], VIDIOC_STREAMOFF, &type)) {
		printf("VIDIOC_STREAMOFF failed\n");
		return (void *)-1;
	} else
		printf("VIDIOC_STREAMOFF ok\n");

	if (-1 == free_frame_buffers(device_id))
		return (void *)-1;
	
	close(fd[device_id]); //close camera
	camera3 =2;
	printf("camera2 close\n");
	
	pthread_exit(NULL);
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
	camera4 = 1;
	//pthread_mutex_lock(&mutex[3]);
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
		//pthread_mutex_unlock(&mutex[3]);
		return (void *)-1;
	} else
		printf("VIDIOC_STREAMON ok\n");
	//pthread_mutex_unlock(&mutex[3]);
	
	while (camera_run) {
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
//		memcpy(NV21IMG[3], gDispTab3[buf.index].vir, NV21SIZE); //r

#endif
		if(record_start){
			memcpy((void *)recordMem[device_id].vir,(void *)buffers3[buf.index].start[0],gDispTab3[buf.index].len);
			flushCache(cdx_memtype, &recordMem[device_id], NULL);
			v4l2_buf[device_id].width = 1280;
			v4l2_buf[device_id].height = 720;
			v4l2_buf[device_id].addrPhyY = recordMem[device_id].phy;
			v4l2_buf[device_id].addrPhyC = recordMem[device_id].phy + v4l2_buf[device_id].width*v4l2_buf[device_id].height;
			v4l2_buf[device_id].addrVirY = recordMem[device_id].vir;
			v4l2_buf[device_id].addrVirC = recordMem[device_id].vir + v4l2_buf[device_id].width*v4l2_buf[device_id].height;
			v4l2_buf[device_id].dmafd = recordMem[device_id].ion_buffer.fd_data.aw_fd;
			v4l2_buf[device_id].timeStamp =  (int64_t) systemTime();
			pRecordCamera[device_id]->dataCallbackTimestamp(v4l2_buf[device_id].timeStamp,CAMERA_MSG_VIDEO_FRAME, 
										(char *)&v4l2_buf[device_id], NULL);	
		}

		if (-1 == ioctl(fd[device_id], VIDIOC_QBUF, &buf)) {
			printf("VIDIOC_QBUF buf.index %d failed\n", buf.index);
			free(buf.m.planes);
			return (void *)-1;
		}

		free(buf.m.planes);
	}
	disp_disable();
	usleep(20*1000);

	if (-1 == ioctl(fd[device_id], VIDIOC_STREAMOFF, &type)) {
		printf("VIDIOC_STREAMOFF failed\n");
		return (void *)-1;
	} else
		printf("VIDIOC_STREAMOFF ok\n");

	if (-1 == free_frame_buffers(device_id))
		return (void *)-1;
	
	close(fd[device_id]); //close camera
	camera4 = 2;
	
	printf("camera3 close\n");
	
	pthread_exit(NULL);
}

void *REC_Camera(void *arg){
	int recordwith[4];
	int recordheith[4];
	int ret;

	sem_wait(&record_sem);

	for(int i=0; i < 4; i++){
		recordwith[i] = config_get_weith(dev_id[i]);
		recordheith[i] = config_get_heigth(dev_id[i]);
		
		pRecordCamera[i] = new RecordCamera(dev_id[i]);
		pRecordCamera[i]->videoEncParmInit(recordwith[i], recordheith[i], 
									recordwith[i], recordheith[i], 8, 25, VENC_CODEC_H264);
									
		pRecordCamera[i]->setCallbacks(Rec_usernotifyCallback,pRecordCamera[i]);
		
		pRecordCamera[i]->setDuration(1 * 60);

		pRecordCamera[i]->start();
		pRecordCamera[i]->startRecord();	

		memset(&recordMem[i], 0, sizeof(dma_mem_des_t));
		ret = allocOpen(cdx_memtype, &recordMem[i], NULL);  
		if (ret < 0) {
			LOGE("ion_alloc_open failed");
		}
		recordMem[i].size = recordwith[i]*recordheith[i]*3/2;
		ret = allocAlloc(cdx_memtype, &recordMem[i], NULL);
		if(ret < 0){
			ALOGE("allocAlloc failed");
		}
	}

	record_start = 1;
	while(1){		
		usleep(10000);
	}
	pthread_exit(NULL);
	for(int i= 0; i < 4; i++){
		allocFree(cdx_memtype, &recordMem[i], NULL);
		pRecordCamera[i]->stopRecord();
		pRecordCamera[i]->stop();
		pRecordCamera[i]->videoEncDeinit();
		delete pRecordCamera[i];
		pRecordCamera[i] = NULL;
	}
	sem_destroy(&record_sem);
}

#define TEST_WIDTH 1280 
#define TEST_HEIGHT 720

int* phrase_input(int argc,char *argv[]){
//	static int input_dev[4];
	if(argc != 6){
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
	int second=0;

#if RECODER_TEST
	sdk_log_setlevel(6);
	DvrRecordManagerInit();

	if(sem_init(&record_sem,0,0) != 0){
		printf("record_sem is Error!\n");
	}
#endif

	if(sem_init(&semabsd, 0, 0) != 0)
	{
		printf("semabsd is Error!\n");
	}
	//install_sig_handler();
	init_eglimg_dmafd();
	init_cameradev(phrase_input(argc,argv));
	
	if(argc > 5 ){
		second = atoi(argv[5]);
		printf("the time is %d\n",second);
		if(second ==0)
			second =100;

		if (createTimer(NULL, &mTimerID, __avm_timer_cb) == 0) {
			setPeriodTimer(second, 0, mTimerID);
		}
		else {
			printf("create timer fail\n");
		}
	}	

	IonAllocOpen();
	input_size.width = TEST_WIDTH;
	input_size.height = TEST_WIDTH;
	
	camera_run =1 ;
	avm_run =1;
	
	pthread_create(&tid360Camera0[0], NULL, CameraDisplay0, NULL);
	pthread_create(&tid360Camera1[0], NULL, CameraDisplay1, NULL);
	pthread_create(&tid360Camera2[0], NULL, CameraDisplay2, NULL);
	pthread_create(&tid360Camera3[0], NULL, CameraDisplay3, NULL);
	pthread_create(&avm_camera,NULL,AVM_Camera,NULL);

#if RECODER_TEST
	pthread_create(&record_camera,NULL,REC_Camera,NULL);
#endif
//	pthread_create(&tidMOD,NULL,Mod_process,NULL);
//	pthread_create(&tidBSD,NULL,Bsd_process,NULL);
//  pthread_create(&tidFCW,NULL,fcw_process,NULL);

	pthread_join(tid360Camera0[0], NULL);
	pthread_join(tid360Camera1[0], NULL);
	pthread_join(tid360Camera2[0], NULL);
	pthread_join(tid360Camera3[0], NULL);

//	pthread_join(tidBSD, NULL);
//	pthread_join(tidMOD, NULL);
//  pthread_join(tidFCW, NULL);
	printf("child thread free\n");

	while (1) {
		usleep(30000);
	}

	return 0;
}

void __avm_timer_cb(sigval_t sig)
{
	printf("timer enter\n");

	camera_run=0;
	avm_run=0;
	usleep(1000*1000);

	printf("camera status %d %d %d %d %d\n",camera1,camera2,camera3,camera4,avm_run);

	
	stopTimer(mTimerID);

	printf("system suspend\n");
	
	system("echo mem > /sys/power/state");

	
	init_eglimg_dmafd();
	camera_run =1 ;
	
	pthread_create(&tid360Camera0[0], NULL, CameraDisplay0, NULL);
	pthread_create(&tid360Camera1[0], NULL, CameraDisplay1, NULL);
	pthread_create(&tid360Camera2[0], NULL, CameraDisplay2, NULL);
	pthread_create(&tid360Camera3[0], NULL, CameraDisplay3, NULL);

	
	usleep(1000*1000);
	
	if(camera1 ==1 && camera2 ==1 && camera3 ==1 && camera4 ==1)
	{
		avm_run=1;
	}
	printf("camera status %d %d %d %d %d\n",camera1,camera2,camera3,camera4,avm_run);

	printf("timer sleep end\n");
}
