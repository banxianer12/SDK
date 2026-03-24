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
#include "sf_xcb_bsd.h"
#include "sf_xcb_fcw.h"
#include "sf_xcb_mod.h"
#include "xcb_GroundView.h"
#include "sunxi_camera_v2.h"
#include "sunxi_display2.h"
#include <assert.h>
#include "DmaIon.h"
#include "xcb_time.h"

/******************** RECODER_TEST **********************/
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
/******************** RECODER_TEST **********************/
#define RECODER_TEST 0 // record test
/**********************************************************************************************/

sem_t record_sem;
pthread_t record_camera;
dma_mem_des_t recordMem[4];
V4L2BUF_t v4l2_buf[4];
int record_start = 0;
RecordCamera *pRecordCamera[4];
unsigned int cdx_memtype = MEM_TYPE_CDX_NEW;

/**********************************************************************************************/

#define USDMA
//旋转
#define ANGLE_STEP  3
#define IDX_TRAJ_FRONT 270/ANGLE_STEP
#define IDX_TRAJ_BACK 90/ANGLE_STEP
#define IDX_TRAJ_LEFT 225/ANGLE_STEP
#define IDX_TRAJ_RIGHT 315/ANGLE_STEP

#define WINDOW_W 1280
#define WINDOW_H 800

#define WINDOW_2D_W 384 // 1280*0.3
#define WINDOW_2D_H 800

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
pthread_t tidMOD;
pthread_t tidBSD;
pthread_t tidFCW;
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
//#define NV61SIZE   1280*720*2

#define CUSTOM_COUNT 1

pthread_t tidAVM[CUSTOM_COUNT];



pthread_mutex_t mutex0 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond0 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond3 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex[4] = {mutex0,mutex1,mutex2,mutex3};
pthread_cond_t cond[4] = {cond0, cond1, cond2, cond3};

int vb_dma[4];


/**********************************************************************************************/
/**********************************************************************************************/
#define __stdlib__ 1
#define MAX_VIDEO_NUM_EGL 4
#define MEDIA_DEVICE "/dev/media0"

struct hw_isp_media_dev *media = NULL;

/**********************************************************************************************/
/**********************************************************************************************/
GLeglImageOES g_elgImage[MAX_VIDEO_NUM_EGL];

EGLDisplay egl_display;
EGLSurface egl_surface;

ViewParameter cameraParameter;
ViewParameter cameraParameter3D;
ViewParameter cameraSingle;
#define NV21SIZE 1382400
GLuint texture_id[8];
unsigned char NV21IMG[4][NV21SIZE];
unsigned char pImg[4][1843200];
unsigned char ADASIMG[MAX_VIDEO_NUM_EGL][NV21SIZE]; 
int ntt = 0;
int NTT = 0;
int pic_duration = 0;
double *p = NULL;
//========logo nosale===========
static avm_ui_type_t logoUIHandle;
static png_texture_t logo_png_texture;


//============handel about stitching and single view==================
avm_stitching_type_t avm_stitching_Handle;
avm_car_type_t avm_car_Handle;
avm_camera_type_t camera_state;
avm_single_type_t avm_single_Handle;

float fGreyRatio[4];


GLeglImageOES elg_Image[MAX_VIDEO_NUM_EGL];

typedef struct eglimg_dmafd_
{
  EGLImageKHR eglimg;
  int dmafd;
} eglimg_dmafd_t;


static eglimg_dmafd_t g_eglimg_dmafd_array[EGLIMG_DMAFD_MAX];

float matrix_F[9] = {-36.001579, -2017.977011, 4010.577319,
                     108.799132, -1945.798321, 3729.453214,
                     0.027623, -0.502215, 1.000000};
float matrix_B[9] = {365.989561, 2283.871423, 3945.201422,
                     187.185748, 2226.842337, 3657.633380,
                     0.047656, 0.576244, 1.000000};

//============================car skin=============================
int nBodyColorIndex = 0;
int nMaxBodyColorNum = 12;
float bodycolor[12][4] = {
    {0.0, 0.0, 0.0, 1.0},
    {0.0, 0.0, 0.7, 1.0},
    {0.35, 0.0, 0.0, 1.0},
    {0.9, 0.75, 0.1, 1.0},
    {0.0, 0.8, 0.0, 1.0},
    {0.5, 0.5, 0.5, 1.0},
    {0.9, 0.5, 0.0, 1.0},
    {0.8, 0.0, 0.0, 1.0},
    {0.75, 0.75, 0.75, 1.0},
    {0.4, 0.2, 0.4, 1.0},
    {0.95, 0.95, 0.95, 1.0},
    {0.9, 0.9, 0.0, 1.0},
};

//============================line===============================
enum GEAR{D,R};
int  changeflag=0;
png_texture_t line_png;
avm_line_static_type_t avmLineStaticHandle;
avm_line_dynamic_single_type_t avmLineDynamicSingleHandle;
avm_line_dynamic_single_type_t avmLineDynamicSingleHandle0;
avm_line_dynamic_single_type_t avmLineDynamicSingleHandle1;

avm_line_dynamic_bird_type_t avmLineDynamicBirdHandle0;
avm_line_dynamic_bird_type_t avmLineDynamicBirdHandle1;
avm_line_dynamic_bird_type_t avmLineDynamicBirdHandle2;

//======================ground view=============================
avm_fbo_transchassis_type_t avm_fbo_transchassis;
void *ground_view_arr;
avm_transchassis_movement_info car_move_info;
GLuint pTexResult;
bool use_ground_view=false;

enum View_name{
default_view,
or_f,
or_b,
l_f_t,
r_f_t,
ud_b,
ud_l,
ud_f,
ud_r,
or_l,
or_r,
sd_f,
sd_b
};
/*
    view=1~10:
    1:original front image
    2:original back image
    3:left front tyre image
    4:right front tyre image
    5:undistorted back image
    6:undistorted left image
    7:undistorted front image
    8:undistorted right image
    9:original left image
    10:original right image
    */



//=========================ADAS======================
static bool MOD_allarm;
static bool BSD_allarm;
static bool FCW_allarm;


//mod
bool which_side_mod_alarm[4]={false,false,false,false};
int mod_ui_window_x=0,mod_ui_window_y=0,mod_ui_window_w=300,mod_ui_window_h=480;//根据俯视图调节
static avm_ui_type_t modUIHandle;
static png_texture_t mod_png_texture;

//bsd
void *allarm_obj,*detect_obj;
adas_auxiliary_type_t bsd_ui_handle;
static sem_t semabsd;
float rect_color[4]={1.0,0.0,1.0,1.0};//rgba


// /** The Position of BSD warning object */
// typedef struct bsd_position_s
// {
//   int x;          //box to stand for obj position x (*目标框左上角横坐标*)
//   int y;          //box to stand for obj position x (*目标框左上角纵坐标*)
//   int w;          //box to stand for obj position w (*目标框宽*)
//   int h;          //box to stand for obj position h (*目标框高*)
//   double world_x; //box to stand for obj position x (*目标距离车侧面物理距离*)
//   double world_y; //box to stand for obj position y (*目标距离车尾物理距离*)
// } bsd_position_t;
// bsd_position_t *position;
#endif /*_avm_H_*/
