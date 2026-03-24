/******************************************************************************

                  XCB 版权所有 (C), 2017-2020,

 ******************************************************************************
  文 件 名   :
  版 本 号   : V1.0
  作    者   :
  生成日期   : 2019年4月25日
  最近修改   :
  功能描述   :
  函数列表   :
  修改历史   :
  1.日    期   : 2019年4月25日
    作    者    : pengtao
    修改内容   : 创建文件

******************************************************************************/
#ifndef _avm_H_
#define _avm_H_


#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <linux/fb.h> 
#include <sys/mman.h> 
#include <sys/ioctl.h> 
#include <sys/time.h>
#include <time.h>
#include <linux/videodev2.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/select.h>
#include <thread>
#include <vector>
#include <sys/inotify.h>
#include <limits.h>
#include <error.h>

#include <iostream>
#include <fstream>
#include <stdio.h>  
#include <signal.h>  
#include <time.h>  
#include <string.h>  
#include <stdlib.h>  
#include <unistd.h>
#include <math.h>

#include "hwdisp2.h"

#include "drm_fourcc.h"


#include "xcb_avmlib.h"
#include "sunxi_camera_v2.h"
#include "sunxi_display2.h"
#include <assert.h>
#include "DmaIon.h"

#define RECODER_TEST 0 // record test

#if RECODER_TEST
#include "aw_ini_parser.h"
#include "CameraDebug.h"


#include "V4L2CameraDevice2.h"
#include "CallbackNotifier.h"
#include "CameraHardware2.h"
#include "audio_hal.h"

#ifndef CDX_V27
#include "log.h"
#endif
#include "vencoder.h"
#include "CdxMuxer.h"
#include "Rtc.h"
#include "StorageManager.h"
#include "DvrFactory.h"
#include "CameraFileCfg.h"
#include <sys/socket.h>
#include <sys/un.h>
#include "Fat.h"
#include "DebugLevelThread.h"
#include "DvrRecordManager.h"
#include "moudle/AutoMount.h"
#endif
/**********************************************************************************************/
/**********************************************************************************************/

#define ISDMA
//旋转
#define ANGLE_STEP  3
#define IDX_TRAJ_FRONT 270/ANGLE_STEP
#define IDX_TRAJ_BACK 90/ANGLE_STEP
#define IDX_TRAJ_LEFT 225/ANGLE_STEP
#define IDX_TRAJ_RIGHT 315/ANGLE_STEP
/**< Index of camera's moving trajectory points
 * only used in 3D view
 */
int g_nIdxTraj = IDX_TRAJ_FRONT;
/**< Number of camera's moving trajectory points
 * (with one circle) only used in 3D view
 */
int g_nNumTraj = 0;

/**< Points coordinate of camera's moving trajectory
 * (with one circle) only used in 3D view
 */
double *g_pTrajPts = NULL;
/**< Index of camera's moving trajectory points
 * for 3D front/back/left/right view
 */
ViewParameter g_tBirdViewParams;
/**< 3D view camera parameter, and ROI on screen
 * for showing */
ViewParameter g_t3DViewParams;
ViewParameter g_tBirdView2DParams;

int turn_around = 0;
int turn_angle = 0;
int step_count = 1;
int refresh_flag = 0;
int Tire_Frame_Step = 0;
int Tire_Frame_Rate=0;

#define GPU_DISPLAY 1

#define CUSTOM_COUNT  	1

pthread_t tid360Camera0[CUSTOM_COUNT];
pthread_t tid360Camera1[CUSTOM_COUNT];
pthread_t tid360Camera2[CUSTOM_COUNT];
pthread_t tid360Camera3[CUSTOM_COUNT];

pthread_t avm_camera;

/**********************************************************************************************/
/**********************************************************************************************/

#define CH_NUM 4

#define CAMERA_CNT 1

#define CAMERA_WIDTH 1280
#define CAMERA_HEIGHT 720

#define DMAFD_ARRAY_NUM 16
#define EGLIMG_DMAFD_MAX 32

/**********************************************************************************************/
/**********************************************************************************************/
int g_count = 0;

/**********************************************************************************************/
/**********************************************************************************************/
#define WIDTH 2560
#define HEIGHT 720
//#define OUT_FMT 6 // nv61:6,nv21:4
//#define SIZE   		1382400  //1382400  1280*720*3/2  //1280*720*2

#define CUSTOM_COUNT 1

pthread_t tidAVM[CUSTOM_COUNT];

//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mutex0 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond0 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond3 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex[4] = {mutex0,mutex1,mutex2,mutex3};
pthread_cond_t cond[4] = {cond0,cond1,cond2,cond3};

// video_buf_t vb0;
// video_buf_t vb1;
// video_buf_t vb2;
// video_buf_t vb3;

int vb_dma[4];

//struct video_buffer *buf = NULL;

/**********************************************************************************************/
/**********************************************************************************************/
#define __stdlib__ 1
#define MAX_VIDEO_NUM_EGL 4
#define MEDIA_DEVICE "/dev/media0"

struct hw_isp_media_dev *media = NULL;

// struct vi_info
// {
//   int Chn;
//   int isp_id;
//   int s32MilliSec;
//   struct video_fmt vfmt;
//   struct video_buffer buffer;
// };

/**********************************************************************************************/
/**********************************************************************************************/
GLeglImageOES g_elgImage[MAX_VIDEO_NUM_EGL];

EGLDisplay egl_display;
EGLSurface egl_surface;

ViewParameter cameraParameter;
ViewParameter cameraParameter3D;
ViewParameter cameraSingle;

GLuint texture_id[8];

unsigned char pImg[4][1843200];
int ntt = 0;
int NTT = 0;
int pic_duration = 0;
double *p = NULL;
////////////stitching
avm_stitching_type_t avm_stitching_Handle;
avm_car_type_t avm_car_Handle;
#if __stdlib__
avm_camera_type_t camera_state;
#endif
avm_single_type_t avm_single_Handle;

float fGreyRatio[4];
////////////////////UI
avm_ui_type_t avmUIHandle;
avm_ui_type_t avmUIHandle_1;
avm_ui_type_t calibUIHandle;
png_texture_t *png_texture = new png_texture_t[1];
png_texture_t *png_texture1 = new png_texture_t[3];
png_texture_t *g_png_calib = new png_texture_t[6];

GLeglImageOES elg_Image[MAX_VIDEO_NUM_EGL];

typedef struct eglimg_dmafd_
{
  EGLImageKHR eglimg;
  int dmafd;
} eglimg_dmafd_t;
unsigned char pixel_cap[MAX_VIDEO_NUM_EGL]
                       [1280 * 720 * 2]; ///////////当前后、左、前、右四幅画面//

static eglimg_dmafd_t g_eglimg_dmafd_array[EGLIMG_DMAFD_MAX];

avm_line_dynamic_single_type_t avmLineDynamicSingleHandle;
avm_line_dynamic_bird_type_t avmLineDynamicBirdHandle;

float matrix_F[9] = {-36.001579, -2017.977011, 4010.577319,
                     108.799132, -1945.798321, 3729.453214,
                     0.027623, -0.502215, 1.000000};
float matrix_B[9] = {365.989561, 2283.871423, 3945.201422,
                     187.185748, 2226.842337, 3657.633380,
                     0.047656, 0.576244, 1.000000};
png_texture_t line_png;

#endif /*_avm_H_*/
