/******************************************************************************

                  xcb 版权所有 (C), 2017-2020,

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
#include "AAVM05A_SDK_Demo.h"
#include "xcb_time.h"
#include <math.h>
#include <time.h>
bool bvideoflag = true;
//////////////
struct KeyState
{
  uint8_t mKbKey;
  uint8_t mKbMode;
};

struct KeyState Keystate;

// key 0:3D ; 1--6(Back-1 Left-2 Front-3 Right-4 Leftback-5 Rightback-6);
// 7:3D+LDW ; 8:3D+BSD; 9:3D+MOD; a:3D+BSD+LDW; b:3D+MOD+LDW
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

/********************************************************************************/
int setup_dmafd2eglimg(EGLDisplay egl_display, int dma_fd, int dmafd_w,
                       int dmafd_h, EGLImageKHR *eglImage, int index)
{
  int atti = 0;
  EGLint attribs[30];
  int ret;
  int i = 0;
  EGLImageKHR img;

  if (dma_fd < 0)
  {
    printf("dmafd parameter err.\n");
    return -1;
  }
  atti = 0;
  attribs[atti++] = EGL_WIDTH;
  attribs[atti++] = CAMERA_WIDTH;
  attribs[atti++] = EGL_HEIGHT;
  attribs[atti++] = CAMERA_HEIGHT;
  attribs[atti++] = EGL_LINUX_DRM_FOURCC_EXT;
  attribs[atti++] = DRM_FORMAT_NV61; // DRM_FORMAT_NV21;
  attribs[atti++] = EGL_DMA_BUF_PLANE0_FD_EXT;
  attribs[atti++] = dma_fd;
  attribs[atti++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
  attribs[atti++] = 0;
  attribs[atti++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
  attribs[atti++] = CAMERA_WIDTH;

  attribs[atti++] = EGL_DMA_BUF_PLANE1_FD_EXT;
  attribs[atti++] = dma_fd;
  attribs[atti++] = EGL_DMA_BUF_PLANE1_OFFSET_EXT;
  attribs[atti++] = CAMERA_WIDTH * CAMERA_HEIGHT;
  attribs[atti++] = EGL_DMA_BUF_PLANE1_PITCH_EXT;
  attribs[atti++] = CAMERA_WIDTH;

  attribs[atti++] = EGL_YUV_COLOR_SPACE_HINT_EXT;
  attribs[atti++] = EGL_ITU_REC709_EXT;
  attribs[atti++] = EGL_SAMPLE_RANGE_HINT_EXT;
  attribs[atti++] = EGL_YUV_FULL_RANGE_EXT;
  attribs[atti++] = EGL_NONE;

  img = eglCreateImageKHR(egl_display, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, 0,
                          attribs);

  printf("index = %d,dma_fd=%d, dmafd_w=%d, dmafd_h=%d \n", index, dma_fd,
         dmafd_w, dmafd_h);

#if 0	
		if (img == EGL_NO_IMAGE_KHR)
		{
			printf("Error %s(): failed: 0x%08X, dmafd=%x\n", __func__, glGetError(), dma_fd);
			break;
		}
#endif
  *eglImage = img;

  return 0;
}

static int search_dmafd(int dmafd, EGLImageKHR *p_img, int *p_index)
{
  int ret = -1;
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

GLeglImageOES bind_dmafd(video_buf_t video_buf, EGLDisplay dpy)
{
  int ret;
  EGLImageKHR img;
  int index;
  int dmaindex = video_buf.channel;
  int dmafd = video_buf.dma_fd;
  int dmafd_w = video_buf.width;
  int dmafd_h = video_buf.height;
  if (search_dmafd(dmafd, &img, &index) != 0)
  {
    ret = setup_dmafd2eglimg(dpy, dmafd, dmafd_w, dmafd_h, &img, dmaindex);
    if (ret != 0)
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

/*********************************************************************************
**********************************************************************************/
void Render_2D()
{
  fGreyRatio[0] = 1;
  fGreyRatio[1] = 1;
  fGreyRatio[2] = 1;
  fGreyRatio[3] = 1;

  camera_state.camera_state[0] = true;
  camera_state.camera_state[1] = true;
  camera_state.camera_state[2] = true;
  camera_state.camera_state[3] = true;
  xcb_avmlib_stitching_TopView_Render(&avm_stitching_Handle, cameraParameter, &camera_state, texture_id, 0, &fGreyRatio[0]);

  xcb_avmlib_car_Render_bottom(&avm_car_Handle, cameraParameter, 0);
  xcb_avmlib_car_Render_2d(&avm_car_Handle, cameraParameter);
  xcb_avm_line_dynamic_bird_Render(&avmLineDynamicBirdHandle0);
}

void Render_3D()
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

  xcb_avmlib_car_Render_bottom(&avm_car_Handle, cameraParameter3D, 1);
  xcb_avmlib_car_Render_3d(&avm_car_Handle, cameraParameter3D, 0.2);
}

void read_img(char *fileName, int FILE_SIZE, unsigned char *buf)
{
  FILE *fp;
  fp = fopen(fileName, "rb");
  if (!fp)
  {
    printf("Open Fail %s!\n", fileName);
  }
  else
  {
    fread(buf, sizeof(unsigned char), FILE_SIZE, fp);
  }
  fclose(fp);
}

void radarinit(avm_single_type_t *avm_single_type)
{
  avm_radar_type.iCarWidth = CARWIDTH;
  avm_radar_type.iCarLength = CARLENTH + 200;
  avm_radar_type.iPosX = -CARWIDTH / 2;
  avm_radar_type.iPosY = CARLENTH / 2;
  xcb_avmlib_InitRadar(&avm_radar_type, RADAR_NUM, 1);
  int radarindex = 0;

  /////////////////////车前方雷达，包含侧面部分
  for (int irow = 0; irow < 6; irow++)
  {
    for (int icol = 0; icol < 4; icol++)
    {
      float basicwidth;
      float basicstartx;
      float basicendx;
      float basicstarty = avm_radar_type.iCarLength / 2 + irow * 100;
      float startytemp = basicstarty;
      float basicendy = avm_radar_type.iCarLength / 2 + irow * 100 + 100;
      if (icol == 0)
      {
        basicwidth = 500;
        basicstartx = -500 - 250;
        basicendx = -250;
        basicstarty = basicstarty + deltay[0];
        basicendy = startytemp + deltay[1];
      }
      else if (icol == 1)
      {
        basicwidth = 250;
        basicstartx = -250;
        basicendx = 0;
        basicstarty = basicstarty + deltay[2];
        basicendy = startytemp + deltay[3];
      }
      else if (icol == 2)
      {
        basicwidth = 250;
        basicstartx = 0;
        basicendx = 250;
        basicstarty = basicstarty + deltay[4];
        basicendy = startytemp + deltay[5];
      }
      else if (icol == 3)
      {
        basicwidth = 250;
        basicstartx = 250;
        basicendx = 250 + 500;
        basicstarty = basicstarty + deltay[6];
        basicendy = startytemp + deltay[7];
      }

      RadarGroup[radarindex].region_cnt = 1;
      RadarGroup[radarindex].radarId = radarindex;
      RadarGroup[radarindex].startPoint[0] = basicstartx;
      RadarGroup[radarindex].startPoint[1] = basicstarty;
      RadarGroup[radarindex].endPoint[0] = basicendx;
      RadarGroup[radarindex].endPoint[1] = basicendy;
      RadarGroup[radarindex].radarPos[0] = 0;
      RadarGroup[radarindex].radarPos[1] = (basicstartx - basicendx) * (basicstartx + basicendx) / 2 / (basicstarty - basicendy) + (basicendy + basicstarty) / 2;
      RadarGroup[radarindex].posType = 0;
      RadarGroup[radarindex].targetState[0] = true;
      RadarGroup[radarindex].space = fradathick;
      RadarGroup[radarindex].high = 800;
      RadarGroup[radarindex].fALPHA = 0.5;
      printf("F1 radarindex:%d y=%f\n", radarindex, RadarGroup[radarindex].radarPos[1]);
      if (irow < 3)
      {
        RadarGroup[radarindex].targetWarnLevel[0] = 0;
      }
      else
      {
        RadarGroup[radarindex].targetWarnLevel[0] = 1;
      }
      radarindex++; //0-23
    }
  }
  /////////////////////车前方雷达，不包含侧面部分
  for (int irow = 6; irow < 9; irow++)
  {
    for (int icol = 1; icol < 3; icol++)
    {
      float basicwidth;
      float basicstartx;
      float basicendx;
      float basicstarty = avm_radar_type.iCarLength / 2 + irow * 100;
      float startytemp = basicstarty;
      float basicendy = avm_radar_type.iCarLength / 2 + irow * 100 + 100;
      // if (icol == 0)
      // {
      //   basicwidth = 500;
      //   basicstartx = -500 - 250;
      //   basicendx = -250;
      //   basicstarty = basicstarty + deltay[0];
      //   basicendy = startytemp + deltay[1];
      // }else
      if (icol == 1)
      {
        basicwidth = 250;
        basicstartx = -250;
        basicendx = 0;
        basicstarty = basicstarty + deltay[2];
        basicendy = startytemp + deltay[3];
      }
      else if (icol == 2)
      {
        basicwidth = 250;
        basicstartx = 0;
        basicendx = 250;
        basicstarty = basicstarty + deltay[4];
        basicendy = startytemp + deltay[5];
      }
      // else if (icol == 3)
      // {
      //   basicwidth = 250;
      //   basicstartx = 250;
      //   basicendx = 250 + 500;
      //   basicstarty = basicstarty + deltay[6];
      //   basicendy = startytemp + deltay[7];
      // }

      RadarGroup[radarindex].region_cnt = 1;
      RadarGroup[radarindex].radarId = radarindex;
      RadarGroup[radarindex].startPoint[0] = basicstartx;
      RadarGroup[radarindex].startPoint[1] = basicstarty;
      RadarGroup[radarindex].endPoint[0] = basicendx;
      RadarGroup[radarindex].endPoint[1] = basicendy;
      RadarGroup[radarindex].radarPos[0] = 0;
      RadarGroup[radarindex].radarPos[1] = (basicstartx - basicendx) * (basicstartx + basicendx) / 2 / (basicstarty - basicendy) + (basicendy + basicstarty) / 2;
      RadarGroup[radarindex].posType = 0;
      RadarGroup[radarindex].targetState[0] = true;
      RadarGroup[radarindex].space = fradathick;
      RadarGroup[radarindex].high = 800;
      RadarGroup[radarindex].fALPHA = 0.5;
      printf("F2 radarindex:%d y=%f\n", radarindex, RadarGroup[radarindex].radarPos[1]);
      if (irow < 2)
      {
        RadarGroup[radarindex].targetWarnLevel[0] = 1;
      }
      else
      {
        RadarGroup[radarindex].targetWarnLevel[0] = 2;
      }

      radarindex++; //24-29
    }
  }

  /////////////////////车后方雷达，包含侧面部分
  for (int irow = 0; irow < 6; irow++)
  {
    for (int icol = 0; icol < 4; icol++)
    {
      float basicwidth;
      float basicstartx;
      float basicendx;
      float basicstarty = -avm_radar_type.iCarLength / 2 - irow * 100;
      float startytemp = basicstarty;
      float basicendy = -avm_radar_type.iCarLength / 2 - irow * 100 - 100;
      if (icol == 0)
      {
        basicwidth = 500;
        basicstartx = -500 - 250;
        basicendx = -250;
        basicstarty = basicstarty - deltay[0];
        basicendy = startytemp - deltay[1];
      }
      else if (icol == 1)
      {
        basicwidth = 250;
        basicstartx = -250;
        basicendx = 0;
        basicstarty = basicstarty - deltay[2];
        basicendy = startytemp - deltay[3];
      }
      else if (icol == 2)
      {
        basicwidth = 250;
        basicstartx = 0;
        basicendx = 250;
        basicstarty = basicstarty - deltay[4];
        basicendy = startytemp - deltay[5];
      }
      else if (icol == 3)
      {
        basicwidth = 250;
        basicstartx = 250;
        basicendx = 250 + 500;
        basicstarty = basicstarty - deltay[6];
        basicendy = startytemp - deltay[7];
      }

      RadarGroup[radarindex].region_cnt = 1;
      RadarGroup[radarindex].radarId = radarindex;
      RadarGroup[radarindex].startPoint[0] = basicstartx;
      RadarGroup[radarindex].startPoint[1] = basicstarty;
      RadarGroup[radarindex].endPoint[0] = basicendx;
      RadarGroup[radarindex].endPoint[1] = basicendy;
      RadarGroup[radarindex].radarPos[0] = 0;
      RadarGroup[radarindex].radarPos[1] = (basicstartx - basicendx) * (basicstartx + basicendx) / 2 / (basicstarty - basicendy) + (basicendy + basicstarty) / 2;
      RadarGroup[radarindex].posType = 2;
      RadarGroup[radarindex].targetState[0] = true;
      RadarGroup[radarindex].space = fradathick;
      RadarGroup[radarindex].high = 800;
      RadarGroup[radarindex].fALPHA = 0.5;
      // printf("B1 radarindex:%d y=%f\n", radarindex, RadarGroup[radarindex].radarPos[1]);
      if (irow < 3)
      {
        RadarGroup[radarindex].targetWarnLevel[0] = 0;
      }
      else
      {
        RadarGroup[radarindex].targetWarnLevel[0] = 1;
      }

      radarindex++; //30-53
    }
  }
  /////////////////////车后方雷达，不包含侧面部分
  for (int irow = 6; irow < 9; irow++)
  {
    for (int icol = 1; icol < 3; icol++)
    {
      float basicwidth;
      float basicstartx;
      float basicendx;
      float basicstarty = -avm_radar_type.iCarLength / 2 - irow * 100;
      float startytemp = basicstarty;
      float basicendy = -avm_radar_type.iCarLength / 2 - irow * 100 - 100;
      // if (icol == 0)
      // {
      //   basicwidth = 500;
      //   basicstartx = -500 - 250;
      //   basicendx = -250;
      //   basicstarty = basicstarty - deltay[0];
      //   basicendy = startytemp - deltay[1];
      // }
      // else
      if (icol == 1)
      {
        basicwidth = 250;
        basicstartx = -250;
        basicendx = 0;
        basicstarty = basicstarty - deltay[2];
        basicendy = startytemp - deltay[3];
      }
      else if (icol == 2)
      {
        basicwidth = 250;
        basicstartx = 0;
        basicendx = 250;
        basicstarty = basicstarty - deltay[4];
        basicendy = startytemp - deltay[5];
      }
      // else if (icol == 3)
      // {
      //   basicwidth = 250;
      //   basicstartx = 250;
      //   basicendx = 250 + 500;
      //   basicstarty = basicstarty - deltay[6];
      //   basicendy = startytemp - deltay[7];
      // }

      RadarGroup[radarindex].region_cnt = 1;
      RadarGroup[radarindex].radarId = radarindex;
      RadarGroup[radarindex].startPoint[0] = basicstartx;
      RadarGroup[radarindex].startPoint[1] = basicstarty;
      RadarGroup[radarindex].endPoint[0] = basicendx;
      RadarGroup[radarindex].endPoint[1] = basicendy;
      RadarGroup[radarindex].radarPos[0] = 0;
      RadarGroup[radarindex].radarPos[1] = (basicstartx - basicendx) * (basicstartx + basicendx) / 2 / (basicstarty - basicendy) + (basicendy + basicstarty) / 2;
      RadarGroup[radarindex].posType = 2;
      RadarGroup[radarindex].targetState[0] = true;
      RadarGroup[radarindex].space = fradathick;
      RadarGroup[radarindex].high = 800;
      RadarGroup[radarindex].fALPHA = 0.5;
      printf("B2 radarindex:%d y=%f\n", radarindex, RadarGroup[radarindex].radarPos[1]);
      if (irow < 2)
      {
        RadarGroup[radarindex].targetWarnLevel[0] = 1;
      }
      else
      {
        RadarGroup[radarindex].targetWarnLevel[0] = 2;
      }

      radarindex++;
    }
  }

  xcb_avmlib_updateRadar_Sector(RadarGroup, RADAR_NUM, avm_single_type);
}

void loadcalibpng()
{

  calibUIHandle.win_x = 0;
  calibUIHandle.win_y = 0;
  calibUIHandle.width = 1280;
  calibUIHandle.height = 720;
  xcb_avmlib_Draw_Png_Init(&calibUIHandle, g_png_calib, 6);
  char pngPath[500];
  char png_fold[300];
  sprintf(png_fold, "./png_yq");
  //标定UI图层----0
  g_png_calib[0].win_x = 640;
  g_png_calib[0].win_y = 360;
  sprintf(pngPath, "%s/%s", png_fold, "calib_ok.png");
  xcb_avmlib_Load_Png(pngPath, g_png_calib[0]);
  g_png_calib[1].win_x = 640;
  g_png_calib[1].win_y = 120;
  sprintf(pngPath, "%s/%s", png_fold, "calib_fail_back.png");
  xcb_avmlib_Load_Png(pngPath, g_png_calib[1]);
  g_png_calib[1].win_x = 640;
  g_png_calib[1].win_y = 120;
  sprintf(pngPath, "%s/%s", png_fold, "calib_fail_front.png");
  xcb_avmlib_Load_Png(pngPath, g_png_calib[2]);
  g_png_calib[1].win_x = 640;
  g_png_calib[1].win_y = 120;
  sprintf(pngPath, "%s/%s", png_fold, "calib_fail_left.png");
  xcb_avmlib_Load_Png(pngPath, g_png_calib[3]);
  g_png_calib[1].win_x = 640;
  g_png_calib[1].win_y = 120;
  sprintf(pngPath, "%s/%s", png_fold, "calib_fail_right.png");
  xcb_avmlib_Load_Png(pngPath, g_png_calib[4]);
  g_png_calib[1].win_x = 640;
  g_png_calib[1].win_y = 480;
  sprintf(pngPath, "%s/%s", png_fold, "calib_ing.png");
  xcb_avmlib_Load_Png(pngPath, g_png_calib[5]);
}
// void calibProcess(bool iscalib)
// {
//   const char *pConfigPath = "./calibData";
//   const char *pAuthFilePath = "xcb_license";
//   const char *pCalibParamsFile = "output";
//   xcb_calib_image_format_t img_format = XCB_CALIB_IMAGE_FORMAT_NV61;
//   calibReturnMsg =
//       xcb_autocalib(pAuthFilePath, pConfigPath, pCalibParamsFile, pImg[0], pImg[1], pImg[2], pImg[3], XCB_CALIB_IMAGE_FORMAT_NV61, imgWidth, imgHeight);
//   if (calibReturnMsg == XCB_CALIB_NO_ERROR)
//   {
//     printf("AutoCalib XCB_CALIB_NO_ERROR,finsh===========\n");
//     xcb_avmlib_Update_Png(g_png_calib[0], &calibUIHandle);
//     xcb_avmlib_Draw_Png(&calibUIHandle);
//     iscalib = true;
//     usleep(200000); // 200ms sleep for show UI
//   }
//   else if (calibReturnMsg == XCB_CALIB_LEFT_FAIL)
//   {
//     xcb_avmlib_Update_Png(g_png_calib[4], &calibUIHandle);
//     printf("AutoCalib XCB_CALIB_LEFT_FAIL===========\n");
//     xcb_avmlib_Draw_Png(&calibUIHandle);
//     iscalib = true;
//     sleep(1);
//   }
//   else if (calibReturnMsg == XCB_CALIB_RIGHT_FAIL)
//   {
//     printf("AutoCalib XCB_CALIB_RIGHT_FAIL===========\n");
//     xcb_avmlib_Update_Png(g_png_calib[5], &calibUIHandle);
//     xcb_avmlib_Draw_Png(&calibUIHandle);
//     iscalib = true;
//     sleep(1);
//   }
//   else if (calibReturnMsg == XCB_CALIB_BACK_FAIL)
//   {
//     printf("AutoCalib XCB_CALIB_BACK_FAIL===========\n");
//     xcb_avmlib_Update_Png(g_png_calib[1], &calibUIHandle);
//     xcb_avmlib_Draw_Png(&calibUIHandle);
//     iscalib = true;
//     sleep(1);
//   }
//   else if (calibReturnMsg == XCB_CALIB_FRONT_FAIL)
//   {
//     printf("AutoCalib XCB_CALIB_FRONT_FAIL===========\n");
//     xcb_avmlib_Update_Png(g_png_calib[2], &calibUIHandle);
//     xcb_avmlib_Draw_Png(&calibUIHandle);
//     iscalib = true;
//     sleep(1);
//   }
//   else if (calibReturnMsg == XCB_CALIB_GET_FRAME_FAIL)
//   {
//     printf("AutoCalib XCB_CALIB_GET_FRAME_FAIL===========\n");
//   }
//   else if (calibReturnMsg == XCB_CALIB_LOAD_CONFIG_FAIL)
//   {
//     printf("AutoCalib XCB_CALIB_LOAD_CONFIG_FAI===========\n");
//   }
//   else if (calibReturnMsg == XCB_CALIB_MATCH_CPUID_FAIL)
//   {
//     printf("AutoCalib XCB_CALIB_MATCH_CPUID_FAIL===========\n");
//   }
//   else if (calibReturnMsg == XCB_CALIB_CONFIG_PATH_EMPTY)
//   {
//     printf("AutoCalib XCB_CALIB_CONFIG_PATH_EMPTY===========\n");
//   }
//   else if (calibReturnMsg == XCB_CALIB_IMAGE_FORMAT_UNSUPPORT)
//   {
//     printf("AutoCalib XCB_CALIB_IMAGE_FORMAT_UNSUPPORT===========\n");
//   }
// }

void *Bsd_process(void *arg)
{
  int rows = 720;
  int cols = 1280;
  int dist_horizontal = 300;
  int dist_vertical = 300;
  float fActSpeed = 30.0f;
  float fCurSpeed = 50.0;
  const char *pCalibOutPath = "./gpu_data/bsd_params";
  const char *pDefaultCalibOutPath = "./default_gpu_data/bsd_params";
  void *pBSD = foryou_xcb_bsd_init(fActSpeed, dist_horizontal, dist_vertical,
                                   cols, rows, pCalibOutPath, pDefaultCalibOutPath);
  sleep(5);
  while (1)
  {
    pthread_mutex_lock(&mutex[0]);
    memcpy(bsdIMG, pImg[0], 1843200); // only use back cameral
    pthread_mutex_unlock(&mutex[0]);
    // printf("bsd process 1\n");
    foryou_xcb_bsd_process(pBSD, bsdIMG, fCurSpeed, &bsdevent);
    //printf("bsd process 2\n");
  }
  foryou_xcb_bsd_release(&pBSD);
  pthread_exit(NULL);
}

///////////////2的俯视图lines///////////加载线使用的图片
void initBirdDynamicLine()
{
  xcb_avm_line_dynamic_bird_Init(&avmLineDynamicBirdHandle0); //注意：初始化之后可以将line_png.texture释放掉此处未释放，根据自己需求
  avmLineDynamicBirdHandle0.iCarLength = 4800;
  avmLineDynamicBirdHandle0.iLineDistance = 2000;
  avmLineDynamicBirdHandle0.iFrontWheelDis = 1000;
  avmLineDynamicBirdHandle0.iRearWheelDis = 1000;
  avmLineDynamicBirdHandle0.iWheelBase = 2800;
  avmLineDynamicBirdHandle0.bDemoorNot[0] = true; //每三根线为一组，对应的标记为true，则显示，否则不显示;
  avmLineDynamicBirdHandle0.bDemoorNot[1] = true;
  avmLineDynamicBirdHandle0.bDemoorNot[2] = true;
  avmLineDynamicBirdHandle0.bDemoorNot[3] = false;
  avmLineDynamicBirdHandle0.fratio[0] = 1.0; //一组三根线，每根线对应的透明度
  avmLineDynamicBirdHandle0.fratio[1] = 1.0;
  avmLineDynamicBirdHandle0.fratio[2] = 1.0;
  avmLineDynamicBirdHandle0.fratio[3] = 0;

  //avmLineDynamicBirdHandle0.line_dynamic_bird->type = 0; //轨迹向前
  avmLineDynamicBirdHandle0.line_dynamic_bird->fMaxAngle = 40.0;
  avmLineDynamicBirdHandle0.line_dynamic_bird->fAngle = 0; //-33.841061,33.549667
  avmLineDynamicBirdHandle0.line_dynamic_bird->pointnum = 30;
  avmLineDynamicBirdHandle0.line_dynamic_bird->thick = 200;
  avmLineDynamicBirdHandle0.line_dynamic_bird->iLineLength = 2500;
  avmLineDynamicBirdHandle0.line_dynamic_bird->bird.view = &cameraParameter;

  xcb_avmlib_Load_Png("./png_images/line.png", line_png);
  avmLineDynamicBirdHandle0.line_dynamic_bird->tex.width = line_png.width;
  avmLineDynamicBirdHandle0.line_dynamic_bird->tex.heigth = line_png.height;
  avmLineDynamicBirdHandle0.line_dynamic_bird->tex.channels = 4;
  avmLineDynamicBirdHandle0.line_dynamic_bird->tex.pixel = line_png.texture;

  xcb_avm_line_dynamic_bird_Gen_VBO(&avmLineDynamicBirdHandle0, &avm_single_Handle);
  xcb_avm_line_dynamic_bird_Update(&avmLineDynamicBirdHandle0, &avm_single_Handle, &avm_car_Handle);

  xcb_avm_line_dynamic_bird_Init(&avmLineDynamicBirdHandle1); //注意：初始化之后可以将line_png.texture释放掉此处未释放，根据自己需求
  avmLineDynamicBirdHandle1.iCarLength = 4800;
  avmLineDynamicBirdHandle1.iLineDistance = 2000;
  avmLineDynamicBirdHandle1.iFrontWheelDis = 1000;
  avmLineDynamicBirdHandle1.iRearWheelDis = 1000;
  avmLineDynamicBirdHandle1.iWheelBase = 2800;
  avmLineDynamicBirdHandle1.bDemoorNot[0] = true; //每三根线为一组，对应的标记为true，则显示，否则不显示;
  avmLineDynamicBirdHandle1.bDemoorNot[1] = true;
  avmLineDynamicBirdHandle1.bDemoorNot[2] = true;
  avmLineDynamicBirdHandle1.bDemoorNot[3] = false;
  avmLineDynamicBirdHandle1.fratio[0] = 0.5; //一组三根线，每根线对应的透明度
  avmLineDynamicBirdHandle1.fratio[1] = 0.5;
  avmLineDynamicBirdHandle1.fratio[2] = 0.5;
  avmLineDynamicBirdHandle1.fratio[3] = 0;

  avmLineDynamicBirdHandle1.line_dynamic_bird->type = 1; //轨迹向前
  avmLineDynamicBirdHandle1.line_dynamic_bird->fMaxAngle = 40.0;
  avmLineDynamicBirdHandle1.line_dynamic_bird->fAngle = -33.841061; //-33.841061,33.549667
  avmLineDynamicBirdHandle1.line_dynamic_bird->pointnum = 30;
  avmLineDynamicBirdHandle1.line_dynamic_bird->thick = 100;
  avmLineDynamicBirdHandle1.line_dynamic_bird->iLineLength = 2500;
  avmLineDynamicBirdHandle1.line_dynamic_bird->bird.view = &cameraParameter;

  xcb_avmlib_Load_Png("./png_images/red2.png", line_png);
  avmLineDynamicBirdHandle1.line_dynamic_bird->tex.width = line_png.width;
  avmLineDynamicBirdHandle1.line_dynamic_bird->tex.heigth = line_png.height;
  avmLineDynamicBirdHandle1.line_dynamic_bird->tex.channels = 4;
  avmLineDynamicBirdHandle1.line_dynamic_bird->tex.pixel = line_png.texture;

  xcb_avm_line_dynamic_bird_Gen_VBO(&avmLineDynamicBirdHandle1, &avm_single_Handle);
  xcb_avm_line_dynamic_bird_Update(&avmLineDynamicBirdHandle1, &avm_single_Handle, &avm_car_Handle);

  xcb_avm_line_dynamic_bird_Init(&avmLineDynamicBirdHandle2); //注意：初始化之后可以将line_png.texture释放掉此处未释放，根据自己需求
  avmLineDynamicBirdHandle2.iCarLength = 4800;
  avmLineDynamicBirdHandle2.iLineDistance = -1600;
  avmLineDynamicBirdHandle2.iFrontWheelDis = 1000;
  avmLineDynamicBirdHandle2.iRearWheelDis = 1000;
  avmLineDynamicBirdHandle2.iWheelBase = 2800;
  avmLineDynamicBirdHandle2.bDemoorNot[0] = false; //每三根线为一组，对应的标记为true，则显示，否则不显示;
  avmLineDynamicBirdHandle2.bDemoorNot[1] = false;
  avmLineDynamicBirdHandle2.bDemoorNot[2] = true;
  avmLineDynamicBirdHandle2.bDemoorNot[3] = false;
  avmLineDynamicBirdHandle2.fratio[0] = 1.0; //一组三根线，每根线对应的透明度
  avmLineDynamicBirdHandle2.fratio[1] = 1.0;
  avmLineDynamicBirdHandle2.fratio[2] = 1.0;
  avmLineDynamicBirdHandle2.fratio[3] = 0;
  avmLineDynamicBirdHandle2.nSingleLineMode = 1;
  avmLineDynamicBirdHandle2.line_dynamic_bird->type = 1; //轨迹向前
  avmLineDynamicBirdHandle2.line_dynamic_bird->fMaxAngle = 40.0;
  avmLineDynamicBirdHandle2.line_dynamic_bird->fAngle = -33.841061; //-33.841061,33.549667
  avmLineDynamicBirdHandle2.line_dynamic_bird->pointnum = 30;
  avmLineDynamicBirdHandle2.line_dynamic_bird->thick = 1600;
  avmLineDynamicBirdHandle2.line_dynamic_bird->iLineLength = 2500;
  avmLineDynamicBirdHandle2.line_dynamic_bird->bird.view = &cameraParameter;

  xcb_avmlib_Load_Png("./png_images/yellow.png", line_png);
  avmLineDynamicBirdHandle2.line_dynamic_bird->tex.width = line_png.width;
  avmLineDynamicBirdHandle2.line_dynamic_bird->tex.heigth = line_png.height;
  avmLineDynamicBirdHandle2.line_dynamic_bird->tex.channels = 4;
  avmLineDynamicBirdHandle2.line_dynamic_bird->tex.pixel = line_png.texture;

  xcb_avm_line_dynamic_bird_Gen_VBO(&avmLineDynamicBirdHandle2, &avm_single_Handle);
  xcb_avm_line_dynamic_bird_Update(&avmLineDynamicBirdHandle2, &avm_single_Handle, &avm_car_Handle);
}
/// end single

////////////////单视图lines///////////////////
void initSingleDynamicLine()
{
  ///////////////单视图动态轨迹lines///////////加载线使用的图片
  xcb_avm_line_dynamic_single_Init(&avmLineDynamicSingleHandle0); //注意：初始化之后可以将line_png.texture释放掉此处未释放，根据自己需求
  float color[4] = {1.0, 0.0, 0.0, 1.0};
  avmLineDynamicSingleHandle0.iCarLength = 5000;
  avmLineDynamicSingleHandle0.iLineDistance = 1800;
  avmLineDynamicSingleHandle0.iFrontWheelDis = 1000;
  avmLineDynamicSingleHandle0.iRearWheelDis = 1000;
  avmLineDynamicSingleHandle0.iWheelBase = 2800;
  avmLineDynamicSingleHandle0.bDemoorNot[0] = true; //每三根线为一组，对应的标记为true，则显示，否则不显示;
  avmLineDynamicSingleHandle0.bDemoorNot[1] = true;
  avmLineDynamicSingleHandle0.bDemoorNot[2] = true;
  avmLineDynamicSingleHandle0.bDemoorNot[3] = false;
  avmLineDynamicSingleHandle0.fratio[0] = 1; //一组三根线，每根线对应的透明度
  avmLineDynamicSingleHandle0.fratio[1] = 1;
  avmLineDynamicSingleHandle0.fratio[2] = 1;
  avmLineDynamicSingleHandle0.fratio[3] = 0;
  //avmLineDynamicSingleHandle0.line_dynamic_single->type = 1;
  avmLineDynamicSingleHandle0.line_dynamic_single->fMaxAngle = 40.0;
  avmLineDynamicSingleHandle0.line_dynamic_single->fAngle = 0; //-33.841061,33.549667
  avmLineDynamicSingleHandle0.line_dynamic_single->pointnum = 30;
  avmLineDynamicSingleHandle0.line_dynamic_single->thick = 100;
  avmLineDynamicSingleHandle0.line_dynamic_single->iLineLength = 2500;

  avmLineDynamicSingleHandle0.line_dynamic_single->single.view = &cameraSingle;

  xcb_avmlib_Load_Png("./png_images/line.png", line_png);
  avmLineDynamicSingleHandle0.line_dynamic_single->tex.width = line_png.width;
  avmLineDynamicSingleHandle0.line_dynamic_single->tex.heigth = line_png.height;
  avmLineDynamicSingleHandle0.line_dynamic_single->tex.channels = 4;
  avmLineDynamicSingleHandle0.line_dynamic_single->tex.pixel = line_png.texture;
  avmLineDynamicSingleHandle0.line_dynamic_single->single.isResived = false;
  xcb_avm_line_dynamic_single_Gen_VBO(&avmLineDynamicSingleHandle0,
                                      &avm_single_Handle);
  xcb_avm_line_dynamic_single_Update(&avmLineDynamicSingleHandle0,
                                     &avm_single_Handle, &avm_car_Handle);

  ///////////////单视图动态轨迹lines///////////加载线使用的图片
  xcb_avm_line_dynamic_single_Init(&avmLineDynamicSingleHandle1); //注意：初始化之后可以将line_png.texture释放掉此处未释放，根据自己需求

  avmLineDynamicSingleHandle1.iCarLength = 4800;
  avmLineDynamicSingleHandle1.iLineDistance = 2000;
  avmLineDynamicSingleHandle1.iFrontWheelDis = 1000;
  avmLineDynamicSingleHandle1.iRearWheelDis = 1000;
  avmLineDynamicSingleHandle1.iWheelBase = 2800;
  avmLineDynamicSingleHandle1.bDemoorNot[0] = true; //每三根线为一组，对应的标记为true，则显示，否则不显示;
  avmLineDynamicSingleHandle1.bDemoorNot[1] = true;
  avmLineDynamicSingleHandle1.bDemoorNot[2] = true;
  avmLineDynamicSingleHandle1.bDemoorNot[3] = false;
  avmLineDynamicSingleHandle1.fratio[0] = 0.2; //一组三根线，每根线对应的透明度
  avmLineDynamicSingleHandle1.fratio[1] = 0.2;
  avmLineDynamicSingleHandle1.fratio[2] = 0.2;
  avmLineDynamicSingleHandle1.fratio[3] = 0;
  avmLineDynamicSingleHandle1.line_dynamic_single->type = 0;
  avmLineDynamicSingleHandle1.line_dynamic_single->fMaxAngle = 40.0;
  avmLineDynamicSingleHandle1.line_dynamic_single->fAngle = -33.841061; //-33.841061,33.549667
  avmLineDynamicSingleHandle1.line_dynamic_single->pointnum = 30;
  avmLineDynamicSingleHandle1.line_dynamic_single->thick = 100;
  avmLineDynamicSingleHandle1.line_dynamic_single->iLineLength = 2500;

  avmLineDynamicSingleHandle1.line_dynamic_single->single.view = &cameraSingle;

  xcb_avmlib_Load_Png("./png_images/red2.png", line_png);
  avmLineDynamicSingleHandle1.line_dynamic_single->tex.width = line_png.width;
  avmLineDynamicSingleHandle1.line_dynamic_single->tex.heigth = line_png.height;
  avmLineDynamicSingleHandle1.line_dynamic_single->tex.channels = 4;
  avmLineDynamicSingleHandle1.line_dynamic_single->tex.pixel = line_png.texture;
  avmLineDynamicSingleHandle1.line_dynamic_single->single.isResived = false;
  xcb_avm_line_dynamic_single_Gen_VBO(&avmLineDynamicSingleHandle1,
                                      &avm_single_Handle);
  xcb_avm_line_dynamic_single_Update(&avmLineDynamicSingleHandle1,
                                     &avm_single_Handle, &avm_car_Handle);
  /// end single
  ///////////////单视图动态轨迹lines///////////加载线使用的图片
  xcb_avm_line_dynamic_single_Init(&avmLineDynamicSingleHandle); //注意：初始化之后可以将line_png.texture释放掉此处未释放，根据自己需求
  avmLineDynamicSingleHandle.iCarLength = 5000;
  avmLineDynamicSingleHandle.iLineDistance = -2000;
  avmLineDynamicSingleHandle.iFrontWheelDis = 1000;
  avmLineDynamicSingleHandle.iRearWheelDis = 1000;
  avmLineDynamicSingleHandle.iWheelBase = 2800;
  avmLineDynamicSingleHandle.bDemoorNot[0] = true; //每三根线为一组，对应的标记为true，则显示，否则不显示;
  avmLineDynamicSingleHandle.bDemoorNot[1] = true;
  avmLineDynamicSingleHandle.bDemoorNot[2] = true;
  avmLineDynamicSingleHandle.bDemoorNot[3] = false;
  avmLineDynamicSingleHandle.fratio[0] = 1; //一组三根线，每根线对应的透明度
  avmLineDynamicSingleHandle.fratio[1] = 1;
  avmLineDynamicSingleHandle.fratio[2] = 1;
  avmLineDynamicSingleHandle.fratio[3] = 0;
  avmLineDynamicSingleHandle.nSingleLineMode = 1;
  avmLineDynamicSingleHandle.line_dynamic_single->type = 0;
  avmLineDynamicSingleHandle.line_dynamic_single->fMaxAngle = 40.0;
  avmLineDynamicSingleHandle.line_dynamic_single->fAngle = 0; //-33.841061,33.549667
  avmLineDynamicSingleHandle.line_dynamic_single->pointnum = 30;
  avmLineDynamicSingleHandle.line_dynamic_single->thick = 2000;
  avmLineDynamicSingleHandle.line_dynamic_single->iLineLength = 2500;

  avmLineDynamicSingleHandle.line_dynamic_single->single.view = &cameraSingle;

  xcb_avmlib_Load_Png("./png_images/yellowold.png", line_png);
  avmLineDynamicSingleHandle.line_dynamic_single->tex.width = line_png.width;
  avmLineDynamicSingleHandle.line_dynamic_single->tex.heigth = line_png.height;
  avmLineDynamicSingleHandle.line_dynamic_single->tex.channels = 4;
  avmLineDynamicSingleHandle.line_dynamic_single->tex.pixel = line_png.texture;
  avmLineDynamicSingleHandle.line_dynamic_single->single.isResived = false;
  xcb_avm_line_dynamic_single_Gen_VBO(&avmLineDynamicSingleHandle,
                                      &avm_single_Handle);
  xcb_avm_line_dynamic_single_Update(&avmLineDynamicSingleHandle,
                                     &avm_single_Handle, &avm_car_Handle);

  /// end single
}

void *AVM_Camera(void *arg)
{
  FILE *fp = NULL;
  int idx = 0;
  ////////////////////////////////////////////////////////////////////////////
  TimeInterval timeNow;
  struct timeval start_linux, end_linux, interval_linux;
  xcb_avmlib_Window_Init(1280, 480, egl_display, egl_surface);
  screen.win_x = 420;
  screen.win_y = 0;
  screen.width = 860;
  screen.height = 480;
  screen.isDMA = true;
  xcb_avmlib_screen_Init(&screen);
  xcb_avmlib_screen_Gen_VBO(&screen);

#ifdef ISDMA
  xcb_avmlib_Gen_TextureBuffer_DMA(4, texture_id);
  avm_stitching_Handle.isDMA = true;
#else
  xcb_avmlib_Gen_TextureBuffer(8, texture_id);
#endif
  printf("DMA texture_id[%u,%u,%u,%u]\n", texture_id[0], texture_id[1],
         texture_id[2], texture_id[3]);
  xcb_avmlib_Gen_TextureBuffer(2, transchassis_texid1);
  xcb_avmlib_Gen_TextureBuffer(2, transchassis_texid2);
  xcb_avmlib_Gen_TextureBuffer(2, transchassis_texid3);

  xcb_avmlib_Gen_TextureBuffer(10, transchassis_textureid);

  xcb_avmlib_stitching_Init(&avm_stitching_Handle, "./xcb_license");

  xcb_avmlib_stitching_Gen_VBO(NULL, "./gpu_data/BowlModel_2D",
                               &avm_stitching_Handle, 0);
  ///读取车底纹图片
  xcb_avmlib_stitching_Gen_VBO(NULL, "./gpu_data/BowlModel",
                               &avm_stitching_Handle, 1);
  ///////////Car
  xcb_avmlib_car_Init(&avm_car_Handle);
  avm_car_Handle.iCarWidth = 1890;
  avm_car_Handle.iCarLength = 4800;

  avm_car_Handle.blind_2d->blind_area_width = 2050;
  avm_car_Handle.blind_2d->blind_area_length = 5000;
  avm_car_Handle.blind_2d->blind_area_cx = 0;
  avm_car_Handle.blind_2d->blind_area_cy = 0;

  avm_car_Handle.blind_3d->blind_area_width = 2050;
  avm_car_Handle.blind_3d->blind_area_length = 5000;
  avm_car_Handle.blind_3d->blind_area_cx = 0;
  avm_car_Handle.blind_3d->blind_area_cy = 0;
  /////////2d bottom
  png_texture_t bottom_2d;
  xcb_avmlib_Load_Png("./png_images/tex_bottom_2d.png", bottom_2d);
  avm_car_Handle.blind_2d->texture_width = bottom_2d.width;
  avm_car_Handle.blind_2d->texture_height = bottom_2d.height;
  avm_car_Handle.blind_2d->channel = 4;
  avm_car_Handle.blind_2d->pixel = bottom_2d.texture;
  ///////////3d bottom
  png_texture_t bottom_3d;
  xcb_avmlib_Load_Png("./png_images/tex_bottom_3d.png", bottom_3d);
  avm_car_Handle.blind_3d->texture_width = bottom_3d.width;
  avm_car_Handle.blind_3d->texture_height = bottom_3d.height;
  avm_car_Handle.blind_3d->channel = 4;
  avm_car_Handle.blind_3d->pixel = bottom_3d.texture;
  ///////设置3d车状态
  avm_car_Handle.car_3d.car_state.swing = true;
  avm_car_Handle.car_3d.car_state.roll = true;
  avm_car_Handle.car_3d.car_state.wheel_axis[0][0] = -769.895;
  avm_car_Handle.car_3d.car_state.wheel_axis[0][1] = 1310.624;
  avm_car_Handle.car_3d.car_state.wheel_axis[0][2] = 350.905;
  avm_car_Handle.car_3d.car_state.wheel_axis[1][0] = 769.895;
  avm_car_Handle.car_3d.car_state.wheel_axis[1][1] = 1310.624;
  avm_car_Handle.car_3d.car_state.wheel_axis[1][2] = 350.905;
  avm_car_Handle.car_3d.car_state.wheel_axis[2][0] = -769.895;
  avm_car_Handle.car_3d.car_state.wheel_axis[2][1] = -1361.619;
  avm_car_Handle.car_3d.car_state.wheel_axis[2][2] = 350.905;
  avm_car_Handle.car_3d.car_state.wheel_axis[3][0] = 769.895;
  avm_car_Handle.car_3d.car_state.wheel_axis[3][1] = -1361.619;
  avm_car_Handle.car_3d.car_state.wheel_axis[3][2] = 350.905;

  //分别设置前左车门，前右车门，后左车门，后右车门，引擎盖、后备箱转动的轴心坐标
  avm_car_Handle.car_3d.car_state.open_axis[0][0] = -871.038;
  avm_car_Handle.car_3d.car_state.open_axis[0][1] = 993.319;
  avm_car_Handle.car_3d.car_state.open_axis[0][2] = 337.764;
  avm_car_Handle.car_3d.car_state.open_axis[1][0] = 871.038;
  avm_car_Handle.car_3d.car_state.open_axis[1][1] = 993.319;
  avm_car_Handle.car_3d.car_state.open_axis[1][2] = 337.764;
  avm_car_Handle.car_3d.car_state.open_axis[2][0] = -872.194;
  avm_car_Handle.car_3d.car_state.open_axis[2][1] = -85.237;
  avm_car_Handle.car_3d.car_state.open_axis[2][2] = 314.237;
  avm_car_Handle.car_3d.car_state.open_axis[3][0] = 854.455;
  avm_car_Handle.car_3d.car_state.open_axis[3][1] = -80.681;
  avm_car_Handle.car_3d.car_state.open_axis[3][2] = 313.001;
  avm_car_Handle.car_3d.car_state.open_axis[4][0] = 1.166;
  avm_car_Handle.car_3d.car_state.open_axis[4][1] = 1269.269;
  avm_car_Handle.car_3d.car_state.open_axis[4][2] = 1028.816;
  avm_car_Handle.car_3d.car_state.open_axis[5][0] = 1.365;
  avm_car_Handle.car_3d.car_state.open_axis[5][1] = -1325.524;
  avm_car_Handle.car_3d.car_state.open_axis[5][2] = 1362.495;
  //分别设置前左车门，前右车门，后左车门，后右车门，引擎盖、后备箱的打开角度
  avm_car_Handle.car_3d.car_state.open_angle[0] = 30;
  avm_car_Handle.car_3d.car_state.open_angle[1] = -30;
  avm_car_Handle.car_3d.car_state.open_angle[2] = 30;
  avm_car_Handle.car_3d.car_state.open_angle[3] = -30;
  avm_car_Handle.car_3d.car_state.open_angle[4] = -30;
  avm_car_Handle.car_3d.car_state.open_angle[5] = 30;
  xcb_avmlib_car_Gen_VBO("./gpu_data/2D_Params", &avm_car_Handle, 0);
  xcb_avmlib_car_Gen_VBO("./gpu_data/3D_Params", &avm_car_Handle, 1);
  xcb_avmlib_car_Gen_VBO("./gpu_data/UnderPrint", &avm_car_Handle, 2);
  ////////////single
#ifdef ISDMA
  avm_single_Handle.isDMA = true;
#endif
  int addnum = 200;

  xcb_avmlib_single_Init(&avm_single_Handle);
  xcb_avmlib_single_Gen_VBO(NULL, "./gpu_data/SingleModel", &avm_single_Handle); //"./auth_data/SingleModel"

  xcb_avmlib_sideView_Gen_VBO(NULL, "./gpu_data/SideViewModelF", &avm_single_Handle, 0);
  xcb_avmlib_sideView_Gen_VBO(NULL, "./gpu_data/SideViewModelB", &avm_single_Handle, 1);
  xcb_avmlib_sideView_Gen_VBO(NULL, "./gpu_data/SideViewModelU", &avm_single_Handle, 2);
  xcb_avmlib_wideAngleView_Gen_VBO(NULL, "./gpu_data/BroadViewModel", &avm_single_Handle);
  cameraParameter.win_x = 10;
  cameraParameter.win_y = 0;
  cameraParameter.win_width = 400;
  cameraParameter.win_height = 480;
  cameraParameter.fov = 80;
  cameraParameter.aspect_ratio = 4.0 / 5.4;
  cameraParameter.ex = 0;
  cameraParameter.ey = 0;
  cameraParameter.ez = 6000;
  cameraParameter.tx = 0;
  cameraParameter.ty = 0;
  cameraParameter.tz = -3000;

  xcb_avmlib_2d_Set_ViewParameter(cameraParameter, 90);

  cameraParameter3D.win_x = 425;
  cameraParameter3D.win_y = 0;
  cameraParameter3D.win_width = 840;
  cameraParameter3D.win_height = 480;
  cameraParameter3D.fov = 35.0;
  cameraParameter3D.aspect_ratio = 84 / 48.0;
  cameraParameter3D.ex = 0;
  cameraParameter3D.ey = 0;
  cameraParameter3D.ez = 3000;
  cameraParameter3D.tx = 0;
  cameraParameter3D.ty = 0;
  cameraParameter3D.tz = 150;
  xcb_avmlib_3d_Set_ViewParameter(cameraParameter3D);

  cameraSingle.win_x = 425;
  cameraSingle.win_y = 0;
  cameraSingle.win_width = 840;
  cameraSingle.win_height = 480;
  cameraSingle.fov = 80.0;
  cameraSingle.aspect_ratio = 84 / 48.0;
  cameraSingle.ex = 0;
  cameraSingle.ey = 0;
  cameraSingle.ez = 800;
  cameraSingle.tx = 0;
  cameraSingle.ty = 0;
  cameraSingle.tz = 0;
  xcb_avmlib_2d_Set_ViewParameter(cameraSingle, 90);

  fp = fopen("./gpu_data/tt", "rb");
  if (!fp)
  {
    printf("the file tt is not exit!\n");
  }
  fread(&ntt, sizeof(int), 1, fp);
  printf("the path PTS NUM:%d", ntt);
  p = (double *)malloc(sizeof(double) * ntt * 2);
  fread(p, sizeof(double), ntt * 2, fp);
  fclose(fp);

  avmUIHandle.win_x = 0;
  avmUIHandle.win_y = 0;
  avmUIHandle.width = 1280;
  avmUIHandle.height = 480;
  xcb_avmlib_Load_Png("./png_images/bg.png", png_texture[0]);
  png_texture[0].win_x = 0;
  png_texture[0].win_y = 0;

  xcb_avmlib_Draw_Png_Init(&avmUIHandle, png_texture, 1);

  avmUIHandle_1.win_x = 0;
  avmUIHandle_1.win_y = 0;
  avmUIHandle_1.width = 1280;
  avmUIHandle_1.height = 480;
  xcb_avmlib_Load_Png("./png_images/MOD_MOV.png", png_texture1[0]);
  png_texture1[0].win_x = 0;
  png_texture1[0].win_y = 100;

  xcb_avmlib_Load_Png("./png_images/track_reverse_dynamic.png", png_texture1[1]);
  png_texture1[1].win_x = 0;
  png_texture1[1].win_y = 270;
  xcb_avmlib_Load_Png("./png_images/track_reverse_static.png", png_texture1[2]);
  png_texture1[2].win_x = 0;
  png_texture1[2].win_y = 360;

  xcb_avmlib_Draw_Png_Init(&avmUIHandle_1, png_texture1, 2);

  xcb_avmlib_Update_Png(png_texture1[2], &avmUIHandle_1);
  iKey = KEYVAL;

  //////////////////////////BSD UI//////////////////////////////////
  BSDUIL.win_x = 420;
  BSDUIL.win_y = 0;
  BSDUIL.width = 860;
  BSDUIL.height = 480;
  xcb_avmlib_Load_Png("./png_images/bsd_warning.png", png_bsd_texture[0]);
  png_bsd_texture[0].win_x = 200;
  png_bsd_texture[0].win_y = 200;
  xcb_avmlib_Draw_Png_Init(&BSDUIL, png_bsd_texture, 1);
  BSDUIR.win_x = 420;
  BSDUIR.win_y = 0;
  BSDUIR.width = 860;
  BSDUIR.height = 480;
  xcb_avmlib_Load_Png("./png_images/bsd_warning.png", png_bsd_texture[0]);
  png_bsd_texture[0].win_x = 660;
  png_bsd_texture[0].win_y = 200;
  xcb_avmlib_Draw_Png_Init(&BSDUIR, png_bsd_texture, 1);
  /////////////////////////////////////////////////////////////////

  // //////////////////俯视图轨迹线///////////////////////
  initBirdDynamicLine();
  // /////////////////单视图动态轨迹线////////////////////
  initSingleDynamicLine();
  // ///////////静态轨迹线设置
  // xcb_avm_line_static_Init(&avmLineStaticHandle); //注意：初始化之后可以将line_png.texture释放掉此处未释放，根据自己需求
  // avmLineStaticHandle.iCarLength = 5000;
  // avmLineStaticHandle.iLineDistance = 2000;
  // png_texture_t line_red;
  // png_texture_t line_yellow;
  // png_texture_t line_green;
  // xcb_avmlib_Load_Png("./png_images/red.png", line_red);
  // xcb_avmlib_Load_Png("./png_images/yellow.png", line_yellow);
  // xcb_avmlib_Load_Png("./png_images/green.png", line_green);
  // avmLineStaticHandle.lines_static->type = 1;
  // avmLineStaticHandle.lines_static->isResived = false;
  // avmLineStaticHandle.lines_static->thick = 80;
  // avmLineStaticHandle.lines_static->scale_length = 400;

  // avmLineStaticHandle.lines_static->stepDistance[0] = 500;
  // avmLineStaticHandle.lines_static->stepDistance[1] = 1500;
  // avmLineStaticHandle.lines_static->stepDistance[2] = 3000;

  // avmLineStaticHandle.lines_static->tex[0].width = line_red.width;
  // avmLineStaticHandle.lines_static->tex[0].heigth = line_red.height;
  // avmLineStaticHandle.lines_static->tex[0].channels = 4;
  // avmLineStaticHandle.lines_static->tex[0].pixel = line_red.texture;

  // avmLineStaticHandle.lines_static->tex[1].width = line_yellow.width;
  // avmLineStaticHandle.lines_static->tex[1].heigth = line_yellow.height;
  // avmLineStaticHandle.lines_static->tex[1].channels = 4;
  // avmLineStaticHandle.lines_static->tex[1].pixel = line_yellow.texture;

  // avmLineStaticHandle.lines_static->tex[2].width = line_green.width;
  // avmLineStaticHandle.lines_static->tex[2].heigth = line_green.height;
  // avmLineStaticHandle.lines_static->tex[2].channels = 4;
  // avmLineStaticHandle.lines_static->tex[2].pixel = line_green.texture;
  // avmLineStaticHandle.lines_static->view = &cameraSingle;

  // xcb_avm_line_static_Gen_VBO(&avmLineStaticHandle, &avm_single_Handle);
  // xcb_avm_line_static_Update(&avmLineStaticHandle, &avm_single_Handle);

  // avm_single_Handle.view = 0;

  // // ////////////////////////////////init process for fbo transchassis/////////////////////////
  // avm_fbo_transchassis_type_t avm_fbo_transchassis; //透明底盘
  // avm_fbo_transchassis.nPatch_Bin_X = 47;           //将底盘水平方向划分为47个格子
  // avm_fbo_transchassis.nPatch_Bin_Y = 45;           //将底盘垂直方向划分为45个格子
  // avm_fbo_transchassis.nChangeIndexornot = 0;       //渲染的这块区域是否需要换图
  // avm_fbo_transchassis.nFirstRender = 1;            //是否为第一次渲染
  // avm_fbo_transchassis.patchsize_height = 10800;    //fbo 背景纹理区域的高
  // avm_fbo_transchassis.patchsize_width = 8000;      //fbo 背景纹理区域的宽
  // avm_fbo_transchassis.window_width = 560;          //显示区域的宽
  // avm_fbo_transchassis.window_height = 480;         //显示区域的高
  // //avm_fbo_transchassis.ISDMA = 0;
  // avm_fbo_transchassis.fBlindAreaWidth = 3000;                                 //底盘区域宽度2350
  // avm_fbo_transchassis.fBlindAreaHeight = 5800;                                //底盘区域高5800
  // avm_fbo_transchassis.fBlindcx = 0;                                           //底盘区域方向偏移（车辆中心为0，0）
  // avm_fbo_transchassis.fBlindcy = -100;                                        //底盘区域下移100mm
  // avm_fbo_transchassis.blending_bin_x = 3;                                     //底盘水平方向增加6个格子的融合区域
  // avm_fbo_transchassis.blending_bin_y = 2;                                     //底盘区域垂直方向增加3个格子的融合区域
  // xcb_avm_fbo_transchassis_Init(&avm_fbo_transchassis, &avm_stitching_Handle); //透明底盘初始化

  // avm_transchassis_car_info avm_trancar_blendinginfor;      //车辆结构信息
  // avm_trancar_blendinginfor.fCar_length = 4800;             //车长
  // avm_trancar_blendinginfor.fCar_width = 1890;              //车宽
  // avm_trancar_blendinginfor.fCar_AxisLenth = 2670;          //车轴长
  // avm_trancar_blendinginfor.fCar_backaxis2back = 1190;      //后悬长
  // avm_trancar_blendinginfor.fCar_lrtyredist = 1540;         //左右轮距
  // avm_trancar_blendinginfor.length_gravity_backaxis = 1335; //重心到后轴长度
  // //avm_trancar_blendinginfor.bind_area;矩形盲区左上，右上，右下 左下 四个顶点x y
  // avm_trancar_blendinginfor.bind_area[0] = avm_car_Handle.blind_2d->blind_area_cx - avm_car_Handle.blind_2d->blind_area_width / 2.0;
  // avm_trancar_blendinginfor.bind_area[1] = avm_car_Handle.blind_2d->blind_area_cy + avm_car_Handle.blind_2d->blind_area_length / 2.0;
  // avm_trancar_blendinginfor.bind_area[2] = avm_car_Handle.blind_2d->blind_area_cx + avm_car_Handle.blind_2d->blind_area_width / 2.0;
  // avm_trancar_blendinginfor.bind_area[3] = avm_car_Handle.blind_2d->blind_area_cy + avm_car_Handle.blind_2d->blind_area_length / 2.0;
  // avm_trancar_blendinginfor.bind_area[4] = avm_car_Handle.blind_2d->blind_area_cx + avm_car_Handle.blind_2d->blind_area_width / 2.0;
  // avm_trancar_blendinginfor.bind_area[5] = avm_car_Handle.blind_2d->blind_area_cy - avm_car_Handle.blind_2d->blind_area_length / 2.0;
  // avm_trancar_blendinginfor.bind_area[6] = avm_car_Handle.blind_2d->blind_area_cx - avm_car_Handle.blind_2d->blind_area_width / 2.0;
  // avm_trancar_blendinginfor.bind_area[7] = avm_car_Handle.blind_2d->blind_area_cy - avm_car_Handle.blind_2d->blind_area_length / 2.0;
  // //avm_trancar_blendinginfor.iou_area;矩形用于判读是否替换图像左上，右上，右下 左下 四个顶点x y
  // avm_trancar_blendinginfor.iou_area[0] = avm_car_Handle.blind_2d->blind_area_cx - avm_car_Handle.blind_2d->blind_area_width / 2.0;
  // avm_trancar_blendinginfor.iou_area[1] = avm_car_Handle.blind_2d->blind_area_cy + avm_car_Handle.blind_2d->blind_area_length / 200.;
  // avm_trancar_blendinginfor.iou_area[2] = avm_car_Handle.blind_2d->blind_area_cx + avm_car_Handle.blind_2d->blind_area_width / 2.0;
  // avm_trancar_blendinginfor.iou_area[3] = avm_car_Handle.blind_2d->blind_area_cy + avm_car_Handle.blind_2d->blind_area_length / 200.;
  // avm_trancar_blendinginfor.iou_area[4] = avm_car_Handle.blind_2d->blind_area_cx + avm_car_Handle.blind_2d->blind_area_width / 2.0;
  // avm_trancar_blendinginfor.iou_area[5] = avm_car_Handle.blind_2d->blind_area_cy - avm_car_Handle.blind_2d->blind_area_length / 200.;
  // avm_trancar_blendinginfor.iou_area[6] = avm_car_Handle.blind_2d->blind_area_cx - avm_car_Handle.blind_2d->blind_area_width / 2.0;
  // avm_trancar_blendinginfor.iou_area[7] = avm_car_Handle.blind_2d->blind_area_cy - avm_car_Handle.blind_2d->blind_area_length / 200.;
  // // //////////////////////////////////init for car move info process///////////////////////////
  // void *ground_view_arr = xcb_avm_transchassis_coordintatetransfer_init(&avm_fbo_transchassis, &avm_trancar_blendinginfor);
  // ///////////////////////////////////////////////////MOD////////////////////////////////////////////
  // int rows = 720;
  // int cols = 1280;
  // int dist_horizontal = 300;
  // int dist_vertical = 300;
  // float fSpeed = 0;
  // float fAngle = 0;
  // bool cameraON[4] = {true, true, true, true};
  // const char *pmod_paramsPath = "./gpu_data/mod_params";
  // const char *pmod_paramsPath2 = "./default_gpu_data/mod_params";
  // void *pMOD = foryou_xcb_mod_init(dist_horizontal, dist_vertical, cols, rows, pmod_paramsPath, pmod_paramsPath2);
  // foryou_xcb_mod_output_t *modoutput = 0;
  // ///////////////////////////////////////////////////BSD////////////////////////////////////////////

  // float fActSpeed = 30.0f;
  // float fCurSpeed = 50.0;
  // foryou_xcb_bsd_output_t *obj = 0, *detectobj = 0;
  // // const char *pCalibOutPath = "./gpu_data/bsd_params";
  // // const char *pDefaultCalibOutPath = "./default_gpu_data/bsd_params";
  // // void *pBSD = foryou_xcb_bsd_init(fActSpeed, dist_horizontal, dist_vertical,
  // //                                  cols, rows, pCalibOutPath, pDefaultCalibOutPath);
  // foryou_xcb_bsd_event_t *bsdevent = 0;
  // ///////////////////////////////////////////////////LDW////////////////////////////////////////////
  // foryou_xcb_ldw_output_t *ldwoutput = NULL;
  // foryou_xcb_ldw_params_t params;
  // params.fStSpeed = 10;
  // params.fActSpeed = 50;
  // params.nCarWidth = 210;
  // params.nWheelbase = 260;

  // const char *pMapPath = "./gpu_data/Ldw2DMappingModel"; //Ldw2DMappingModel_newcar";
  // const char *pMapPath2 = "./default_gpu_data/Ldw2DMappingModel";
  // void *pLDW = foryou_xcb_ldw_init(&params, pMapPath, pMapPath2);
  // ////////////////////////////////////////////////////light_avg//////////////////////////////////////
  light_init_param param;
  param.adjust_frequency = 30;
  param.carLength = 5.0;
  param.carwidth = 2.05;
  param.imgHeight = 720;
  param.imgWidth = 1280;
  param.roih = 1.;
  param.roiw = 1.;
  param.getVal_method = useRect;
  adjust_lum_out_t lumoutput;
  void *pLightavg;
  float ftheta = 45 * 3.1415926 / 180;

  pLightavg = initlightavg("./gpu_data/mod_params", "./gpu_data/mod_params", param);
  //////////为了优化更新亮度均衡的速度，在初始化部分调用该函数，用于提前把一些变量计算好，正常只需要调用一次就行。ftheta改变时，要调用该函数
  xcb_avmlib_stitching_LightRatioInit(ftheta, &avm_stitching_Handle, 0);
  xcb_avmlib_stitching_LightRatioInit(ftheta, &avm_stitching_Handle, 1);
  bool iscalib = false;
  const char *frames_rear = "./HY/back";
  const char *frames_left = "./HY/left";
  const char *frames_front = "./HY/front";
  const char *frames_right = "./HY/right";
  loadcalibpng();
  radarinit(&avm_single_Handle);
  fp = fopen("./gpu_data/B_NV61", "rb");
  fread(pImg[0], sizeof(unsigned char), 1843200, fp);
  fclose(fp);
  fp = fopen("./gpu_data/L_NV61", "rb");
  fread(pImg[1], sizeof(unsigned char), 1843200, fp);
  fclose(fp);
  fp = fopen("./gpu_data/F_NV61", "rb");
  fread(pImg[2], sizeof(unsigned char), 1843200, fp);
  fclose(fp);
  fp = fopen("./gpu_data/R_NV61", "rb");
  fread(pImg[3], sizeof(unsigned char), 1843200, fp);
  fclose(fp);
  EGLSyncKHR sync;
  while (1)
  {
    KeyboardInputProcess(Keystate);

    //if (elg_Image)
    // if (pImg[0] && pImg[1] && pImg[2] && pImg[3])
    {
#ifdef ISDMA
      pthread_cond_wait(&cond[0], &mutex[0]);
      pthread_cond_wait(&cond[1], &mutex[1]);
      pthread_cond_wait(&cond[2], &mutex[2]);
      pthread_cond_wait(&cond[3], &mutex[3]);
      for (int i = 0; i < 4; i++)
      {
        xcb_avmlib_Update_TextureBuffer_DMA(elg_Image[i], texture_id, i);
      }

#else

      // xcb_avmlib_Update_TextureBuffer(imgWidth, imgHeight, pImg[0], pImg[1], pImg[2], pImg[3],
      //                                 texture_id);

      xcb_avmlib_Update_TextureBuffer(pImg[0], pImg[1], pImg[2], pImg[3], texture_id);
#endif

      xcb_avmlib_Clean_Screen(0.1, 0.1, 0.1, 0.5);

      camera_state.camera_state[0] = true;
      camera_state.camera_state[1] = true;
      camera_state.camera_state[2] = true;
      camera_state.camera_state[3] = true;

      ////////////////////////////////////////////////////////亮度均衡//////////////////////////////////////

      light_avg_process(pLightavg, pImg[0], pImg[1], pImg[2], pImg[3], lumoutput);

      float fRatioL[2], fRatioR[2];
      fRatioL[0] = lumoutput.LFAdjustVal;
      fRatioL[1] = lumoutput.LBAdjustVal;
      fRatioR[0] = lumoutput.RFAdjustVal;
      fRatioR[1] = lumoutput.RBAdjustVal;
      //float angle = (output.LineAngle * 3.1415926 / 180);

      float halfw = param.carwidth * 0.5;
      float halfH = param.carLength * 0.5;
      // if (idx % 5 == 0)
      xcb_avmlib_stitching_UpdateLightRatio(ftheta, lumoutput.backAdjustVal, fRatioL, lumoutput.frontAdjustVal, fRatioR,
                                            &avm_stitching_Handle, 0);

      /////////////////////////////////////////1D单视图////////////////////////////////////////////////
      if (iKey == 1)
      {
        //single view front
        avm_single_Handle.view = 1;
#if __stdlib__
        xcb_avmlib_singleView_Render(&avm_single_Handle, cameraSingle, &camera_state, texture_id);
#else
        xcb_avmlib_single_Render(&avm_single_Handle, cameraSingle, texture_id);
#endif

        /////////////////////////////////////////draw 1D front dynamic line////////////////////////////////////////////////

        //avmLineDynamicSingleHandle.line_dynamic_single->type = 0;
        avmLineDynamicSingleHandle0.line_dynamic_single->type = 0;
        avmLineDynamicBirdHandle0.line_dynamic_bird->type = 0; //轨迹向前

        if (fabs(avmLineDynamicBirdHandle0.line_dynamic_bird->fAngle) < 2)
        {
          avmLineDynamicBirdHandle0.line_dynamic_bird->fAngle = 0;
          avmLineDynamicBirdHandle0.bDemoorNot[2] = false;
        }
        else
        {

          avmLineDynamicBirdHandle0.bDemoorNot[2] = true;
        }

        xcb_avm_line_dynamic_bird_Update(&avmLineDynamicBirdHandle0, &avm_single_Handle, &avm_car_Handle);
        xcb_avm_line_dynamic_single_Update(&avmLineDynamicSingleHandle0, &avm_single_Handle, &avm_car_Handle);
        //xcb_avm_line_dynamic_single_Update(&avmLineDynamicSingleHandle, &avm_single_Handle, &avm_car_Handle);

        xcb_avm_line_dynamic_single_Render(&avmLineDynamicSingleHandle0);
        //xcb_avm_line_dynamic_single_Render(&avmLineDynamicSingleHandle);
        // //xcb_avmlib_Use(avmLineDynamicSingleHandle1.prog);
        // xcb_avm_line_dynamic_single_Render(&avmLineDynamicSingleHandle1);
        // radarinit(&avm_single_Handle);
        // xcb_avmlib_RenderRadar_Sector_SingleView(&avm_radar_type,
        //                                          cameraSingle,
        //                                          &RadarGroup[0], RADAR_NUM, 1);
      }
      XCB_TIME_START("line dynamic")

      if (iKey == 2)
      {
        //single view back
        avm_single_Handle.view = 2;
#if __stdlib__
        xcb_avmlib_singleView_Render(&avm_single_Handle, cameraSingle, &camera_state, texture_id);
#else
        xcb_avmlib_single_Render(&avm_single_Handle, cameraSingle, texture_id);
#endif

        if (pic_duration <= 800)
        {
          if (pic_duration % 1 == 0)
          {
            if (avmLineDynamicSingleHandle.line_dynamic_single->fAngle <=
                -avmLineDynamicSingleHandle.line_dynamic_single->fMaxAngle)
            {
              avmLineDynamicSingleHandle.line_dynamic_single->fAngle = avmLineDynamicSingleHandle.line_dynamic_single->fMaxAngle;
              avmLineDynamicSingleHandle0.line_dynamic_single->fAngle = avmLineDynamicSingleHandle0.line_dynamic_single->fMaxAngle;
              avmLineDynamicSingleHandle1.line_dynamic_single->fAngle = avmLineDynamicSingleHandle1.line_dynamic_single->fMaxAngle;
            }
            avmLineDynamicSingleHandle.line_dynamic_single->fAngle -= 2.5;
            xcb_avm_line_dynamic_single_Update(&avmLineDynamicSingleHandle, &avm_single_Handle, &avm_car_Handle);
            avmLineDynamicSingleHandle0.line_dynamic_single->fAngle -= 2.5;
            xcb_avm_line_dynamic_single_Update(&avmLineDynamicSingleHandle0, &avm_single_Handle, &avm_car_Handle);
            avmLineDynamicSingleHandle1.line_dynamic_single->fAngle -= 2.5;
            xcb_avm_line_dynamic_single_Update(&avmLineDynamicSingleHandle1, &avm_single_Handle, &avm_car_Handle);

            if (avmLineDynamicBirdHandle0.line_dynamic_bird->fAngle <=
                -avmLineDynamicBirdHandle0.line_dynamic_bird->fMaxAngle)
            {
              avmLineDynamicBirdHandle0.line_dynamic_bird->fAngle =
                  avmLineDynamicBirdHandle0.line_dynamic_bird->fMaxAngle;
              avmLineDynamicBirdHandle1.line_dynamic_bird->fAngle =
                  avmLineDynamicBirdHandle1.line_dynamic_bird->fMaxAngle;
              avmLineDynamicBirdHandle2.line_dynamic_bird->fAngle =
                  avmLineDynamicBirdHandle2.line_dynamic_bird->fMaxAngle;
            }
            avmLineDynamicBirdHandle0.line_dynamic_bird->fAngle -= 2.5;
            xcb_avm_line_dynamic_bird_Update(&avmLineDynamicBirdHandle0, &avm_single_Handle, &avm_car_Handle);

            avmLineDynamicBirdHandle1.line_dynamic_bird->fAngle -= 2.5;
            xcb_avm_line_dynamic_bird_Update(
                &avmLineDynamicBirdHandle1, &avm_single_Handle, &avm_car_Handle);

            avmLineDynamicBirdHandle2.line_dynamic_bird->fAngle -= 2.5;
            xcb_avm_line_dynamic_bird_Update(
                &avmLineDynamicBirdHandle2, &avm_single_Handle, &avm_car_Handle);
          }
        }
        else
        {
          pic_duration = 0;
        }

        avmLineDynamicBirdHandle0.line_dynamic_bird->type = 1; //后退
        avmLineDynamicSingleHandle.line_dynamic_single->type = 1;
        avmLineDynamicSingleHandle0.line_dynamic_single->type = 1;
        xcb_avm_line_dynamic_single_Update(&avmLineDynamicSingleHandle0, &avm_single_Handle, &avm_car_Handle);
        xcb_avm_line_dynamic_single_Update(&avmLineDynamicSingleHandle, &avm_single_Handle, &avm_car_Handle);
        xcb_avm_line_dynamic_bird_Update(&avmLineDynamicBirdHandle0, &avm_single_Handle, &avm_car_Handle);

        xcb_avm_line_dynamic_single_Render(&avmLineDynamicSingleHandle0);
        xcb_avm_line_dynamic_single_Render(&avmLineDynamicSingleHandle);
        //xcb_avm_line_static_Render(&avmLineStaticHandle, 1);
        // // radarinit(&avm_single_Handle);
        // xcb_avmlib_RenderRadar_Sector_SingleView(&avm_radar_type,
        //                                          cameraSingle,
        //                                          &RadarGroup[0], RADAR_NUM, 2);
        //foryou_xcb_bsd_process(pBSD, pImg[0], fCurSpeed, &bsdevent);
        if (bsdevent)
        {
          switch (*bsdevent)
          {
          case FORYOU_XCB_BSD_Left_Warning: //find obj  in car left  side (*车左边发现目标*)
            //printf("bsd left warning\n");
            // xcb_avmlib_Update_Png(png_bsd_texture[0], &BSDUIR);
            //xcb_avmlib_Use(BSDUIL.prog);
            xcb_avmlib_Draw_Png(&BSDUIL);
            break;
          case FORYOU_XCB_BSD_Right_Warning: //find obj  in car Right  side (*车右边发现目标*)
            //printf("bsd right warning\n");
            //xcb_avmlib_Use(BSDUIR.prog); // use shader program
            xcb_avmlib_Draw_Png(&BSDUIR);
            break;
          case FORYOU_XCB_BSD_Left_Right_Warning:
            //printf("bsd both side warning\n");

            xcb_avmlib_Draw_Png(&BSDUIR);
            //xcb_avmlib_Use(BSDUIL.prog);
            xcb_avmlib_Draw_Png(&BSDUIL);
            break;
          default:
            //printf("bsd no warning\n");
            break;
          }
        }
        else
        {
          printf("bsd no run\n");
        }
      }
      XCB_TIME_END

      if (iKey == 3)
      {
        //side view front
        xcb_avmlib_sideView_Render(&avm_single_Handle, cameraSingle, &camera_state, texture_id, 0, true, true);
        // radarinit(&avm_single_Handle);
      }
      if (iKey == 4)
      {
        //side view back
        xcb_avmlib_sideView_Render(&avm_single_Handle, cameraSingle, &camera_state, texture_id, 1, true, true);
      }
      if (iKey == 5)
      {
        //xcb_avmlib_full_screen_Render(&screen, texture_id, 0); //single view 0 back 1 left 2 front 3 right
        xcb_avmlib_four_screen_Render(&screen, texture_id);
      }
      // if (iKey == 6)
      // { ////////////////////////////////////////////透明底盘/////////////////////////////////////////////
      //   avm_transchassis_movement_info avm_move_info;
      //   if (idx < 500)
      //   {
      //     avm_move_info.ftyrev[0] = -2.250000;
      //     avm_move_info.ftyrev[1] = -2.250000;
      //     avm_move_info.ftyrev[2] = -2.250000;
      //     avm_move_info.ftyrev[3] = -2.250000;
      //   }
      //   if (idx >= 500)
      //   {
      //     avm_move_info.ftyrev[0] = 5.250000;
      //     avm_move_info.ftyrev[1] = 5.250000;
      //     avm_move_info.ftyrev[2] = 5.250000;
      //     avm_move_info.ftyrev[3] = 5.250000;
      //   }
      //   avm_move_info.tyretheta = 0;
      //   avm_move_info.timeinteval = (long long)50;
      //   GLuint pTexResult;
      //   XCB_TIME_START("trans time");
      //   xcb_avm_transchassis_coordintatetransfer_update(ground_view_arr, &avm_move_info, &avm_fbo_transchassis);
      //   xcb_avm_fbo_transchassis_Update(&avm_fbo_transchassis);
      //   ///////////////////////////当运行距离超过一定距离后重置///////////////////
      //   pTexResult = xcb_avm_fbo_transchassis_Render(&avm_fbo_transchassis,
      //                                                &avm_stitching_Handle, cameraParameter,
      //                                                texture_id);
      //   xcb_avm_fbo_transchassis_Texture_Renderwithviewpara2D(&avm_fbo_transchassis,
      //                                                         cameraParameter,
      //                                                         &pTexResult);

      //   xcb_avmlib_stitching_TopView_Render(&avm_stitching_Handle,
      //                                       cameraParameter3D, &camera_state, texture_id, 1,
      //                                       &fGreyRatio[0]);

      //   xcb_avm_fbo_transchassis_Texture_Renderwithviewpara3D(&avm_fbo_transchassis,
      //                                                         cameraParameter3D,
      //                                                         &pTexResult);
      //   XCB_TIME_END
      // }

      //////////////////////////////////////////render2d////////////////////////////////////////////////

      Render_2D();

      // xcb_avmlib_RenderRadar_Sector_2D(&avm_radar_type,
      //                                  cameraParameter,
      //                                  &RadarGroup[0], RADAR_NUM);
      /////////////////////////////////////////draw 2d dynamic line////////////////////////////////////////////////

      //////////////////////////////////////////render3d////////////////////////////////////////////////
      //if (iKey == 6 || iKey == 7)
      {

        if (NTT >= ntt)
        {
          NTT = 0;
        }
        cameraParameter3D.ex = p[NTT * 2];
        cameraParameter3D.ey = p[NTT * 2 + 1];
        float rollAngle = idx % 360;

        avm_car_Handle.car_3d.car_state.roll_angle = rollAngle;

        xcb_avmlib_wheelState_update(&avm_car_Handle);
        xcb_avmlib_3d_Set_ViewParameter(cameraParameter3D);
        Render_3D();

        // xcb_avmlib_RenderRadar_Sector_3D(&avm_radar_type,
        //                                  cameraParameter3D,
        //                                  &RadarGroup[0], RADAR_NUM);
      }

      //////////////////////////////LDW process///////////////////////////////
      // if (iKey == 9)
      // {
      //   long long timeStampMs = timeNow.getCurrentTime();
      //   foryou_xcb_ldw_process(pLDW,
      //                          pImg[0], pImg[1],
      //                          pImg[2], pImg[3],
      //                          fCurSpeed, fAngle,
      //                          timeStampMs,
      //                          &ldwoutput);
      //   if (ldwoutput)
      //   {
      //     if (ldwoutput->left_allarm)
      //       printf("car depature to left\n");
      //     if (ldwoutput->right_allarm)
      //       printf("car depature to right\n");
      //   }
      // }
      // /////////////////////////////MOD process////////////////////////////////
      // if (iKey == 0)
      // {
      //   foryou_xcb_mod_process(pMOD,
      //                          pImg[0], pImg[1],
      //                          pImg[2], pImg[3],
      //                          0, 0,
      //                          FORYOU_XCB_MOD_D_GEAR, cameraON, &modoutput);

      //   if (modoutput)
      //   {

      //     if (modoutput[0].nObjNum > 0)
      //     {
      //       printf("back allarm\n");
      //     }
      //     if (modoutput[1].nObjNum > 0)
      //     {
      //       printf("Left allarm\n");
      //     }
      //     if (modoutput[2].nObjNum > 0)
      //     {
      //       printf("Front allarm\n");
      //     }
      //     if (modoutput[3].nObjNum > 0)
      //     {
      //       printf("Right allarm\n");
      //     }
      //   }
      // }
      idx++;
      NTT++;
      pic_duration++;

      xcb_avmlib_swapbuffer(egl_display, egl_surface);
    }
    // sync = eglCreateSyncKHR(egl_display, EGL_SYNC_FENCE_KHR, NULL);
    // eglClientWaitSyncKHR(egl_display, sync, EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, 3000000000);
    // eglDestroySyncKHR(egl_display, sync);
  }
  pthread_exit(NULL);
}

static void *loop_cap(void *pArg)
{
  struct vi_info *privCap = (struct vi_info *)pArg;
  struct isp_video_device *video = NULL;
  struct buffers_pool *pool = NULL;
  int i = 0;

  if (privCap->Chn >= HW_VIDEO_DEVICE_NUM ||
      NULL == media->video_dev[privCap->Chn])
  {
    printf("vi channel number is invalid!\n", privCap->Chn);
    return NULL;
  }
  else
  {
    video = media->video_dev[privCap->Chn];
  }

  pool = buffers_pool_new(video);
  if (NULL == pool)
    return NULL;

  if (video_req_buffers(video, pool) < 0)
    return NULL;

  video_get_fmt(video, &privCap[privCap->Chn].vfmt);
  for (i = 0; i < privCap[privCap->Chn].vfmt.nbufs; i++)
    video_queue_buffer(video, i);

  if (video_stream_on(video) < 0)
    return NULL;

  video_buf_t vb;
  // int iiiii=0;
  struct timeval camera_time;
  while (1)
  {
    // iiiii++;
    if (video_wait_buffer(video, privCap->s32MilliSec) < 0)
      continue;
    if (video_dequeue_buffer(video, &privCap->buffer) < 0)
      goto disablech;
    buf = &video->pool->buffers[privCap->buffer.index];
    if (privCap->Chn == 0)
      memcpy(pImg[privCap->Chn], buf->planes[0].mem, 1280 * 720 * 2);
      // if (privCap->Chn == 1)
      //   memcpy(pImg[privCap->Chn], buf->planes[0].mem, 1280 * 720 * 2);
      // if (privCap->Chn == 2)
      //   memcpy(pImg[privCap->Chn], buf->planes[0].mem, 1280 * 720 * 2);
      // if (privCap->Chn == 3)
      //   memcpy(pImg[privCap->Chn], buf->planes[0].mem, 1280 * 720 * 2);
#if 1
    vb.channel = privCap->Chn;
    vb.width = 1280;
    vb.height = 720;
    vb.phy_addr = buf->planes[0].mem_phy;
    vb.dma_fd = buf->planes[0].dma_fd;
#ifdef ISDMA
    pthread_mutex_lock(&mutex[privCap->Chn]);
    elg_Image[privCap->Chn] = bind_dmafd(vb, egl_display);
    pthread_cond_signal(&cond[privCap->Chn]);
    pthread_mutex_unlock(&mutex[privCap->Chn]);
#endif

#endif
    bvideoflag = false;
    if (video_queue_buffer(video, privCap->buffer.index) < 0)
      goto disablech;
  }
disablech:

  if (video_stream_off(video) < 0)
    return NULL;
  if (video_free_buffers(video) < 0)
    return NULL;
  buffers_pool_delete(video);
vi_exit:
  return NULL;
}

static void isp_server_start(int isp_id)
{
  isp_init(isp_id);
  isp_run(isp_id);
}

int Camera_Loop(int ch_num, int width, int height, int out_fmt)
{
  pthread_t thid[MAX_VIDEO_NUM];
  int ret, i, ch = -1;
  struct vi_info privCap[MAX_VIDEO_NUM];

  if (media == NULL)
  {
    media = isp_md_open(MEDIA_DEVICE);
    if (media == NULL)
    {
      printf("unable to open media device %s\n", MEDIA_DEVICE);
      return -1;
    }
  }
  else
  {
    printf("mpi_vi already init\n");
  }

  media_dev_init();

  if (ch_num > MAX_VIDEO_NUM)
    ch_num = MAX_VIDEO_NUM;

  for (i = 0; i < ch_num; i++)
  {

    memset(&privCap[i], 0, sizeof(struct vi_info));
    privCap[i].Chn = i;
    privCap[i].s32MilliSec = 8000;
    privCap[i].vfmt.wdr_mode = 2;
    privCap[i].vfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    privCap[i].vfmt.memtype = V4L2_MEMORY_DMABUF;

    switch (out_fmt)
    {
    case 0:
      privCap[i].vfmt.format.pixelformat = V4L2_PIX_FMT_SBGGR8;
      break;
    case 1:
      privCap[i].vfmt.format.pixelformat = V4L2_PIX_FMT_YUV420M;
      break;
    case 2:
      privCap[i].vfmt.format.pixelformat = V4L2_PIX_FMT_YUV420;
      break;
    case 3:
      privCap[i].vfmt.format.pixelformat = V4L2_PIX_FMT_NV12M;
      break;
    case 4:
      privCap[i].vfmt.format.pixelformat = V4L2_PIX_FMT_NV21;
      break;
    case 5:
      privCap[i].vfmt.format.pixelformat = V4L2_PIX_FMT_NV16;
      break;
    case 6:
      privCap[i].vfmt.format.pixelformat = V4L2_PIX_FMT_NV61;
      break;
    default:
      privCap[i].vfmt.format.pixelformat = V4L2_PIX_FMT_YUV420M;
      break;
    }

    privCap[i].vfmt.format.field = V4L2_FIELD_NONE;
    privCap[i].vfmt.format.width = width;
    privCap[i].vfmt.format.height = height;
    privCap[i].vfmt.nbufs = 8;
    privCap[i].vfmt.nplanes = 3;
    privCap[i].vfmt.fps = 25;
    privCap[i].vfmt.capturemode = V4L2_MODE_VIDEO;
    privCap[i].vfmt.use_current_win = 0;

    if (isp_video_open(media, i) < 0)
    {
      printf("isp_video_open vi%d failed\n", i);
      return -1;
    }

    if (video_set_fmt(media->video_dev[i], &privCap[i].vfmt) < 0)
    {
      printf("video_set_fmt failed\n");
      return -1;
    }

    video_get_fmt(media->video_dev[i], &privCap[i].vfmt);

    privCap[i].isp_id = video_to_isp_id(media->video_dev[i]);
    if (privCap[i].isp_id == -1)
      continue;

    ret = pthread_create(&thid[i], NULL, loop_cap, (void *)&privCap[i]);
    if (ret < 0)
    {
      printf("pthread_create loop_cap Chn[%d] failed.\n", i);
      continue;
    }

    if (i % 2)
      isp_server_start(privCap[i].isp_id);
  }

  pthread_join(tidAVM[0], NULL);
  pthread_join(thid[ch_num - 1], NULL);

  return 0;
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

int main(int argc, char *argv[])
{
  int ret = -1;
  int ch_num = CH_NUM;
  int width = WIDTH;
  int height = HEIGHT;
  int out_fmt = OUT_FMT;
  mutex[0] = PTHREAD_MUTEX_INITIALIZER;
  mutex[1] = PTHREAD_MUTEX_INITIALIZER;
  mutex[2] = PTHREAD_MUTEX_INITIALIZER;
  mutex[3] = PTHREAD_MUTEX_INITIALIZER;
  cond[0] = PTHREAD_COND_INITIALIZER;
  cond[1] = PTHREAD_COND_INITIALIZER;
  cond[2] = PTHREAD_COND_INITIALIZER;
  cond[3] = PTHREAD_COND_INITIALIZER;
  init_eglimg_dmafd();

  printf("Compiled:[%s][%s] ch_num=%d, width=%d, height=%d, out_fmt=%d \n",
         __DATE__, __TIME__, ch_num, width, height, out_fmt);

  if (pthread_mutex_init(&mutex[0], NULL) != 0 ||
      pthread_mutex_init(&mutex[1], NULL) != 0 ||
      pthread_mutex_init(&mutex[2], NULL) != 0 ||
      pthread_mutex_init(&mutex[3], NULL) != 0)
  {
    printf("mutex init error \n");
  }

  if (pthread_cond_init(&cond[0], NULL) != 0 ||
      pthread_cond_init(&cond[1], NULL) != 0 ||
      pthread_cond_init(&cond[2], NULL) != 0 ||
      pthread_cond_init(&cond[3], NULL) != 0)
  {
    printf("cond init error \n");
  }

  pthread_create(&tidAVM[0], NULL, AVM_Camera, NULL);
  printf("create bsd TH\n");
  pthread_create(&g_thdBSD, NULL, Bsd_process, NULL); //盲区监测
  Camera_Loop(CH_NUM, WIDTH, HEIGHT, OUT_FMT);

  pthread_join(g_thdBSD, NULL);
  printf("finish bsd TH\n");
  pthread_mutex_destroy(&mutex[0]);
  pthread_mutex_destroy(&mutex[1]);
  pthread_mutex_destroy(&mutex[2]);
  pthread_mutex_destroy(&mutex[3]);

  return 0;
}
