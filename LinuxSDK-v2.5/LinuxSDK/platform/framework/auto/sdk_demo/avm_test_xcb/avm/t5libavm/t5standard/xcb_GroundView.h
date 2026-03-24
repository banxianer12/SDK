#pragma once
#ifndef __GROUND_VIEW_H__
#define __GROUND_VIEW_H__
#include "xcb_avmlib.h"
//
//#include <EGL/egl.h>
//#include <GLES2/gl2.h>
//#define GL_GLEXT_PROTOTYPES
//#include <GLES2/gl2ext.h>
//#define EGL_EGLEXT_PROTOTYPES
//#include <EGL/eglext.h>
///////////////////////////////透明底盘，FBO版本/////////////////////////////
// typedef struct
// {
//   int vertex_number;   // 模型顶点数
//   int tri_face_number; //三角片元总数
//   float *pPosition;
//   float *pCord;
//   float *pOutPosition; ////////有运动估计算法，根据pPosition中的数据更新pOutPosition位置
//   GLuint fbo_Framebuffer;
//   GLuint fbo_Texturebuffer[2];
//   GLuint buffers[5]; // vertex buffer
//   int nRenderIndex;  //texture buffer中用于渲染的标记texture的index
// } Patch_transchassis_fbo;
// typedef struct
// {
//   Patch_transchassis_fbo *fbo_transchassis_patch;
//   GLuint prog;            // shader program，纹理贴图shader
//   GLuint Stitchingprog;   // shader program，环视拼接贴图shader
//   float patchsize_width;  //整个物理坐标系的宽度，单位mm
//   float patchsize_height; //整个物理坐标系的高度，单位mm
//   int window_width;       //窗口的宽度，或者说texture的宽度
//   int window_height;      //窗口的高度，或者说texture的高度
//   int nPatch_Bin_X;       //用于构建底纹x方向上网格点数量
//   int nPatch_Bin_Y;       //用于构建底纹y方向上网格点数量
//   int nFirstRender;       //是否是第一次渲染
//   int nChangeIndexornot;  //是否进行切换渲染texuture操作
// } avm_fbo_transchassis_type_t;

typedef struct
{
  float fCar_length;        //车长(car length )
  float fCar_width;         //长宽(car width)
  float fCar_backaxis2back; //后弦(tyre to rare length)
  float fCar_AxisLenth;     //轴距(front back axi length)
  float fCar_lrtyredist;    //左右后轮距离(right left tyre dist)
  float bind_area[8];
  float iou_area[8];             //用于判断是否保存图像所设置的iou比较区域
  float length_gravity_backaxis; //////////////重心到后轴长度 1335
} avm_transchassis_car_info;

typedef struct
{
  float ftyrev[4];       // velocity of tyre,FLV FRV BLV BRV
  float tyretheta;       // FLA FRA
  long long timeinteval; // interval times (ms)
} avm_transchassis_movement_info;

typedef struct movement_info_list
{
  avm_transchassis_movement_info *movement_info;
  int numMovementInfo;
  /* data */
} avm_transchassis_movement_info_list;

///////////初始化坐标转换模块///////////////////////
void *xcb_avm_transchassis_coordintatetransfer_init(avm_fbo_transchassis_type_t *avm_fbo_transchassis,
                                                    avm_transchassis_car_info *avm_car_infor);
/*
该函数的功能：
1.依据输入的轮速、轮胎转角计算当前车辆的X、Y以及偏航角（注意这里的坐标系一般取后轴中心为原点）；
2.选择之前保存的合适的图像以及对应的坐标系、偏航角，假设对应的时刻为t；（时间最近，同时图像能够覆盖透明底盘区域）
3.通过当前时刻的位置以及偏航角，计算当前的透明底盘的物理区域（注意透明底盘的物理区域坐标是依据车辆中心的坐标原点计算）在t时刻时对应的物理位置，
avm_fbo_transchassis_type_t *avm_fbo_transchassis.fbo_transchassis_patch.pVerTexOutPos中；
return:
*/
int xcb_avm_transchassis_coordintatetransfer_update(
    void *ground_view_arr,                            //初始化得到的车辆运动模型
    avm_transchassis_movement_info *avm_move_info,    ////////////////当前车辆运动的轮速以及转角
    avm_fbo_transchassis_type_t *avm_fbo_transchassis //根据车辆的运动信息更新渲染的
);
////////////重置当前坐标系//////////////////////////////
int xcb_avm_transchassis_reset(void *ground_view_arr_, avm_fbo_transchassis_type_t *avm_fbo_transchassis);

////////////释放计算资源////////////////////////////////
int xcb_avm_transchassis_exit(void *ground_view_arr_);

#endif // !__GROUND_VIEW_H__