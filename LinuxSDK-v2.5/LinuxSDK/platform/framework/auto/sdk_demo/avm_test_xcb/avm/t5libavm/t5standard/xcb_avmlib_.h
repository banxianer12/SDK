/*3D avm sdk
version:1.1
create time:2019.11.24
modify time:
author:kangli shi
function:
camera data :DMA(Direct Memory Access)
1.2d bird view  //Note:unit :millimeter
2.3d surround view //Note:unit :millimeter
3.single view
4.ui //Note:unit :pixle
*/
#ifndef XCB_AVMLIB_H_
#define XCB_AVMLIB_H_
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2ext.h>
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext.h>
#include <fbdev_window.h>
////////define struct

///////////////////////////////////////////////////////////////////
////////////////////基础数据结构以及基础功能///////////////////////
typedef char unint_8;
typedef struct {
  //int id;
  int vertex_number;   // 模型顶点数
  int tri_face_number; //三角片元总数
  GLuint *pbuffers;    // vertex buffer
  // id（顶点坐标+4*纹理坐标+4*混合权重+三角片元索引）
  int verTexGroupNum[4];
  int tri_face_groupNum[4];
  float *pPos[4]; //////////////4个摄像头的位置信息
  float *pWei[4];
  GLuint *texture_id;
  void *pData; // added by zliu for update virtual camera 20200315
} Patch;

//////////used by car and radar
typedef struct {
  int id;
  int vertex_number;   // 模型顶点数
  int tri_face_number; //三角片元总数
  GLuint *buffers;     // vertex buffer
                   // id（顶点坐标+4*纹理坐标+4*混合权重+三角片元索引）
  GLuint *texture_id;
  void *pData; // added by zliu for update virtual camera 20200315
} Patch_Lines;
/////////////////////描述摄像头的状态
typedef struct {
  bool camera_state[4] = {true, true, true,
                          true}; /////BLFR,四个摄像头的好坏状态
} avm_camera_type_t;

typedef struct {
  int win_x; // view port: starting position x coordinate ,Note:on the screen
  int win_y; // view port: starting position y coordinate ,Note:on the screen
  int win_width;        // view port: width,Note:unit pixel
  int win_height;       // view port: height,Note:unit pixel
  float ex, ey, ez;     // the camera's  position (x,y,z)
  float tx, ty, tz;     // the camera's direction target position(x,y,z)
  float fov;            // filed of view
  float aspect_ratio;   // perspective aspect_ratio=width/height
  float mvp_Matrix[16]; // mvp_Matrix= model matrix * view matrix * perspective
                        // // matrix
} ViewParameter;

typedef struct {
  int win_x;               // the x coordate display on the window
  int win_y;               // the y coordate display on the window
  int width;               // the image's width
  int height;              // the image's height
  GLenum format = GL_RGBA; // default format
  int channels;
  unsigned char *texture;
} png_texture_t;
/*init shader interface
const char *v_shader:vertex shader
const char *f_shader:fragmment shader
*/
GLuint xcb_avmlib_Shader_Init(const char *v_shader, const char *f_shader);
// delete shader program
void xcb_avmlib_Shader_Delete(GLuint program);
/*unuse DMA texture
 */
void xcb_avmlib_Gen_TextureBuffer(int cnt, GLuint *bufferId);
/*use DMA texture
int cnt:the number of texture buffer id;
GLuint *bufferId//store texture buffer id
*/
void xcb_avmlib_Gen_TextureBuffer_DMA(int cnt, GLuint *bufferId);
void xcb_avmlib_Use(GLuint program);   // use shader program
void xcb_avmlib_Unuse(GLuint program); // unuse shader program
void xcb_avmlib_Clean_Screen();        // clean screen
void xcb_avmlib_2d_Set_ViewParameter(ViewParameter &viewParameter); //
void xcb_avmlib_3d_Set_ViewParameter(ViewParameter &viewParameter); //
/*@param
ViewParameter &viewParameter：视口、相机位置及目标点位置的参数
float angle:相机up方向的朝向与坐标轴x轴的夹角，
其中坐标轴的x的正方向指向车的右侧，
y轴正方向指向车前方，夹角是按照与x轴的夹角，按照逆时针方向
*/
void xcb_avmlib_2d_Set_ViewParameter(ViewParameter &viewParameter, float angle);
void xcb_avmlib_3d_Set_ViewParameter(ViewParameter &viewParameter,
                                     float rotateAngle_X,
                                     float rotateAngle_Y,
                                     float rotateAngle_Z); //
//////////////
/*xcb_avmlib_Window_Init interface
int Width:screen's width
int Height:screen's height
EGLDisplay &egl_display:display id or handle on system
EGLSurface &egl_surface:system window or frame buffer handle
return value: -1:fail,0:succeed
*/
int xcb_avmlib_Window_Init(int Width, int Height, EGLDisplay &egl_display,
                           EGLSurface &egl_surface);
/*display picture interface
EGLDisplay &egl_display:display id or handle on system
EGLSurface &egl_surface:system window or frame buffer handle
*/
void xcb_avmlib_swapbuffer(EGLDisplay &egl_display, EGLSurface &egl_surface);
/*
float r:红色分量，取值范围0.0-1.0
float g:绿色分量，取值范围0.0-1.0
float b:蓝色分量，取值范围0.0-1.0
float a:透过率分量，取值范围0.0-1.0
*/
void xcb_avmlib_Clean_Screen(float r, float g, float b,
                             float a); // clean screen
/////////////非DMA模式下的数据更新功能
void xcb_avmlib_Update_TextureBuffer(int width, int height,
                                     const unsigned char *pixelRear,
                                     const unsigned char *pixelLeft,
                                     const unsigned char *pixelFront,
                                     const unsigned char *pixelRight,
                                     GLuint *g_texture_id);

void xcb_avmlib_Update_TextureBuffer(const unsigned char *pixelRear,
                                     const unsigned char *pixelLeft,
                                     const unsigned char *pixelFront,
                                     const unsigned char *pixelRight,
                                     GLuint *g_texture_id);

/*
pixel_index：
0:rear camera,
1:left camera,
2:front camera,
3:right camera
*/
void xcb_avmlib_Update_TextureBuffer_Single(const unsigned char *pixel,
                                            int pixel_index,
                                            GLuint *g_texture_id);
//////////////////////////////////
/////////////////DMA
/*
GLeglImageOES p_Img: image data
avm_stitching_type_t *avm_stitching:stitching variable
int index:the camera's index
Note:
index=0,back camera
index=1,left camera
index=2,front camera
index=3,right camera
return value: -1:fail,0:succeed
*/
int xcb_avmlib_Update_TextureBuffer_DMA(GLeglImageOES p_Img,
                                        GLuint *g_texture_id, int index);

////////////////////////////////////////////Drawpng/////////////////////
///////////////////////ui贴图相关
typedef struct {
  int win_x;           // 窗口显示起始位置X坐标
  int win_y;           //窗口显示起始位置Y坐标
  int width;           //窗口显示的宽
  int height;          //窗口显示的高
  int vertex_number;   //
  int tri_face_number; //
  GLuint buffers[3];   //
  GLuint texture_id;
  GLuint prog;
} avm_ui_type_t;
int xcb_avmlib_Load_Png(const char *filename, png_texture_t &png);
int xcb_avmlib_Draw_Png_Init(avm_ui_type_t *avm_ui_type,
                             png_texture_t *png_texture, int cnt);
int xcb_avmlib_Draw_Png_Exit(avm_ui_type_t *avm_ui_type);
int xcb_avmlib_Draw_Png(avm_ui_type_t *avm_ui_type);
int xcb_avmlib_Update_Png(png_texture_t &png_texture,
                          avm_ui_type_t *avm_ui_type);

////////////////////////fbo for ui/////////////////////////////
typedef struct {
  int win_x;           // ui画布在显示窗口中的左上角x坐标，input
  int win_y;           // ui画布在显示窗口中的左上角y坐标，input
  int width;           // ui画布在显示窗口中的宽，input
  int height;          // ui画布在显示窗口中的高，input
  int vertex_number;   //
  int tri_face_number; //
  GLuint buffers[3];   //
  GLuint texture_id;   // ui画布的texture id，属于返回值
  GLuint png_texture_id; //用于更新ui的png图片id，不需要外部创建
  GLuint prog;
  GLuint fboprog;
  void *fboObject;
} avm_fbo_ui_type_t;

int xcb_avmlib_fbo_ui_Init(avm_fbo_ui_type_t *avm_ui_type);
int xcb_avmlib_fbo_ui_Exit(avm_fbo_ui_type_t *avm_ui_type);
////////////渲染ui画布，非fbo
int xcb_avmlib_fbo_ui_Render(avm_fbo_ui_type_t *avm_ui_type);

////////////清空渲染的画布，fbo
int xcb_avmlib_fbo_ui_Clear(avm_fbo_ui_type_t *avm_ui_type);
//将png图片通过fbo方式渲染到ui画布上，mode：0，采用blending模式；1，直接替换对应区域模式
//结构png_texture中的位置为其在画布中的位置和宽高，而不是在窗口中的，这个请务必注意
int xcb_avmlib_fbo_ui_Update_Png(png_texture_t &png_texture,
                                 avm_fbo_ui_type_t *avm_ui_type, int mode = 0);
int xcb_avmlib_fbo_ui_Clear_Png(png_texture_t &png_texture,
                                avm_fbo_ui_type_t *avm_ui_type);
///////////////////////////////////////////////////////////////

////////////////////功能1：拼接相关功能////////////////////////////////////
/////////////////stitching////////////////
typedef struct {
  Patch *stitching;
  GLuint prog;        // shader program
  int mode = 0;       // 0:unfold,1:fold
  bool isDMA = false; // false:not support DMA;true:support DMA;
} avm_stitching_type_t;
/*initialize stitching variable
const char* auth_file: license file path
return value: -1:fail,0:succeed
*/
int xcb_avmlib_stitching_Init(avm_stitching_type_t *avm_stitching_type,
                              const char *auth_file);
int xcb_avmlib_stitching_Exit(avm_stitching_type_t *avm_stitching_type);

/*
const char *filename:the file's name of stitching
const char *default_filename:the default file's name of stitching(
    if "filename" cannot access,"default_filename" will be used )
avm_stitching_type_t *avm_stitching:stitching variable
int type:type=0,stitching 2d; type=1,stitching 3d;

return value: -1:fail,0:succeed
*/
int xcb_avmlib_stitching_Gen_VBO(const char *filename,
                                 const char *default_filename,
                                 avm_stitching_type_t *avm_stitching_type,
                                 int type);

/*
avm_stitching_type_t *avm_stitching: stitching variable
ViewParameters viewParameters:view port and camera parameters variable
GLuint *texture_id:texture buffer id.Note:single view and stitching all use the
texture buffer id int type:type=0,stitching 2d; type=1,stitching 3d; Note:
type=0,viewParameters set 2d
type=1,viewParameters set 3d
*/
void xcb_avmlib_stitching_TopView_Render(
    avm_stitching_type_t *avm_stitching_type, ViewParameter &viewParameter,
    avm_camera_type_t *camera, GLuint *g_texture_id, int index,
    float *fGreyRatio1);

int xcb_avmlib_stitching_ObtainBlindArea(
    float &fHalfW, float &fHalfH, avm_stitching_type_t *avm_stitching_type,
    int type);

/////////////////////功能1.1：拼接中的亮度均衡
/*
avm_stitching_type_t *avm_stitching: stitching variable
ftheta, 用于计算相邻图像增加亮度程度所在直线的直线角度；
fRatioB，后方需要增加的亮度值，需要除以255进行归一化
fRatioL，左上[0]和左下[1]对应的需要增加的亮度值，需要除以255进行归一化
fRatioF，前方需要增加的亮度值，需要除以255进行归一化
fRatioR，右上[0]和右下[1]对应的需要增加的亮度值，需要除以255进行归一化
各种拼接模式下都需要调用该函数
*/
//////////为了优化更新亮度均衡的速度，在初始化部分调用该函数，用于提前把一些变量计算好，正常只需要调用一次就行。ftheta改变时，要调用该函数
int xcb_avmlib_stitching_LightRatioInit(
    float &ftheta, avm_stitching_type_t *avm_stitching_type, int type);
int xcb_avmlib_stitching_UpdateLightRatio(
    float &ftheta, float fRatioB, float fRatioL[2], float fRatioF,
    float fRatioR[2], avm_stitching_type_t *avm_stitching_type, int type);

/////////////////////功能2：渲染车模//////////////////////////////
////////////////////////////////////render car
/// function////////////////////////////////

typedef struct {
  int id;
  int vertex_number;   // 模型顶点数
  int tri_face_number; //三角片元总数
  GLuint buffers[3];   // vertex buffer             //
                     // id（顶点坐标+4*纹理坐标+4*混合权重+三角片元索引）
  GLuint texture_id;
} Car;

typedef struct {
  float blind_area_width = 0;  //盲区区域的宽//
  float blind_area_length = 0; //盲区区域的长//
  float blind_area_cx = 0;     //盲区区域的宽offset
  float blind_area_cy = 0;     //盲区区域的长offset
  int texture_width;           //盲区区域映射纹理的图片的宽//
  int texture_height;          //盲区区域设置纹理的图片的高//
  int channel;                 //盲区区域设置纹理的图片通道数//
  unsigned char *pixel;        //指向盲区区域设置纹理的像素//
  Car *bottom;                 //指向车底部盲区区域的buffer指针,//
} Blind_Area;
/*define car state struct
@param wheel_axis input:(0~3):left front wheel,right front wheel,left rear
wheel,right rear wheel coordinate
@param roll input:the default wheel is not rolling
@param swing input:the default wheel is not swinging
@param roll_angle input:rolling angle
@param swing_angle input:swing angle
@param open_axis input:(0~5)left front door,right front door,left rear
door,right rear door,hood and trunk coordinate
@param open_angle input:(0~5) open angle
@param wheel_matrix output:wheel matrix
@param open_matrix output:door matrix
*/

typedef struct {
  float wheel_axis[4][3]; // 4(0~3):left front wheel,right front wheel,left rear
                          // wheel,right rear wheel,3(0~2):coordinate x,y,z
  bool roll = false;      // the default wheel is not rolling
  bool swing = false;     // the default wheel is not swinging
  float model_matrix[4][16];
  float roll_angle = 0;  // rolling angle
  float swing_angle = 0; // swing angle
} avm_wheel_type_t;
typedef struct {
  float
      door_axis[4]
               [3]; // 4(0~3)设置车门轴的坐标，顺序分别是前左，前右，后左，后右
  float angle[4]; //设置车门打开的角度，分别按照前左、前右，后左，后右的顺序
  float model_matrix[4][16]; //输出值
} avm_door_type_t;

typedef struct {
  int iCarWidth;        // 2d car's width
  int iCarLength;       // 2d car's length
  int iBlindAreaWidth;  //设置盲区区域的宽
  int iBlindAreaLength; //设置盲区区域
  Blind_Area *blind_2d; //指向2d盲区区域的指针
  Blind_Area *blind_3d; //指向3d盲区区域的指针  
  Patch_Lines *car_2d;        //注意，初始化开辟空间，退出释放空间
  struct Car_3D {
    Patch_Lines *car;             //初始化开辟资源，退出释放资源
    GLuint *texture_id;     //初始化开辟资源，退出释放资源
    avm_wheel_type_t wheel; //指向轮胎的指针
    avm_door_type_t door;   //指向车门的指针
  } car_3d;
  Patch_Lines *car_bottom;    //注意，初始化开辟空间，退出释放空间，
  int texture_width;    //设置底部纹理图片的宽
  int texture_height;   //设置底部纹理图片的高
  int texture_channels; //设置底部纹理的通道数
  unsigned char *
      pixel; //指针指向底部纹理的像素存储地址，注意底部纹理值传给GPU后，可释放底部纹理占用的空间
  GLuint prog; // shader program
} avm_car_type_t;

typedef struct {
  int type; // type=0,car 2d;type=1,car 3d;
  int id; // skin id :type =0 ,id=0 car 2d;type=1&id=0: car 3d body skin,type=1
          // & id=1:the tyre's skin of 3d car
  char name[260];
  GLenum format; // Note:type=0, format=RGBA;type=1,format=RGB;
  int texture_width;
  int texture_height;
  unsigned char *texture; //
} avm_car_skin_type_t;

///////////car//////////////
/*@param
input：
char *srcPath:默认文件的路径名
char *dstPath:resize后的文件的路径名
float resize_width:输入resize后的车模的宽,单位毫米
float resize_length:输入resize后的车模的长,单位毫米
float resize_height: 输入resize后的车模的高,单位毫米
bool isTexture=true:resize的默认文件是否带纹理，默认状态下的文件是车模和纹理都有
int iTextureNum = 2:车模纹理的个数
int iVertNum = 5:车模的部件的个数
output：
float &src_width:默认车模型的宽,单位毫米
float &src_length:默认车模型的长,单位毫米
float &src_height:默认车模型的高,单位毫米
注意：
*/
int xcb_avmlib_3DCarmodel_Resize(char *srcPath, char *dstPath,
                                       float &src_width, float &src_length,
                                       float &src_height, float resize_width,
                                       float resize_length, float resize_height,
                                       bool isTexture = true,
                                       int iTextureNum = 2, int iVertNum = 5);

int xcb_avmlib_3DCarmodel_ObtainOrignalSize(float &src_width,
                                                  float &src_length,
                                                  float &src_height);

int xcb_avmlib_3DCarmodel_move(float fx, float fy, float fz);
/*initialize car variable
return value: -1:fail,0:succeed
*/
int xcb_avmlib_car_Init(const char *auth_file,
                              avm_car_type_t *avm_car_type, char *file_path);
int xcb_avmlib_car_Exit(avm_car_type_t *avm_car_type);
/*
lightRGB:AmbientLight RGB,
example:float lightRGB[3]
R value:0.0~1.0
g value:0.0~1.0
B value:0.0~1.0
*/
void xcb_avmlib_car_SetAmbientLight(float *lightRGB);

/*
update material
pMaterialName: name of material
fColor: rgba of material. the value is in [0-1].
*/
int xcb_avmlib_UpdateMeterial(char *pMaterialName, float fColor[4]);

void xcb_avmlib_car_TurnLeftLightOn(float yellowcolor[4]);
void xcb_avmlib_car_TurnLeftLightOff();
void xcb_avmlib_car_TurnRightLightOn(float yellowcolor[4]);
void xcb_avmlib_car_TurnRightLightOff();

/*
fReflectance: 反光率，0-1.0之间，越大反光越厉害
*/
void xcb_avmlib_car_SetReflectanceRatio(float fReflectance);
/*
获得某个材质使用贴图还是材质RGBA进行渲染的。例如，车灯有时使用RGBA进行渲染，则此时修改对应材质的RGBA值可以实现车灯闪烁。
而当材质使用贴图渲染时，需要更新对应名称的贴图，和之前换肤换车灯一种方式
返回值：nMaterialAttribute: 0,
RGBA材质进行渲染；1，纹理贴图进行渲染。贴图名称需要固定，参考模型定义文献。
*/
void xcb_avmlib_car_ObtainMaterialAttributeofDemonstrate(
    char *file_name, int &nMaterialAttribute);
/*

avm_car_type_t *avm_car_type:car variable
int type:type=0,car 2d; type=1,car 3d;type=2,car bottom texture;

return value: -1:fail,0:succeed
*/
int xcb_avmlib_car_Gen_VBO(const char *filename,
                                 avm_car_type_t *avm_car_type, int type);
int xcb_avmlib_car_Update_3D_CarModel(const char *filename,
                                            avm_car_type_t *avm_car_type);
int xcb_avmlib_car_Update_SkinTexture(
    avm_car_type_t *avm_car_type, avm_car_skin_type_t &avm_car_skin_type);
/*
int iMode: 车灯换肤的情况下，0代表默认车灯，1代表新车灯
*/
int xcb_avmlib_car_Update_LightTexture(
    avm_car_type_t *avm_car_type, avm_car_skin_type_t &avm_car_skin_type,
    int iMode);
/*Render interface
 */
void xcb_avmlib_car_Render_2d(avm_car_type_t *avm_car_type,
                                    ViewParameter &viewParameter);
void xcb_avmlib_car_Render_3d(avm_car_type_t *avm_car_type,
                                    ViewParameter &viewParameter);
void xcb_avmlib_car_Render_bottom(avm_car_type_t *avm_car_type,
                                        ViewParameter &viewParameter);
void xcb_avmlib_car_2d_Set_ViewParameter(ViewParameter &viewParameter,
                                               float up_x, float up_y,
                                               float up_z);
//////////更新车的状态：
/*
bool wheel = false// 表示是否更新车轮的状态，默认状态不更新
bool door = false//表示是否更新车门的状态，默认状态不更新
*/
int xcb_avmlib_carState_Update(avm_car_type_t *avm_car_type,
                                     bool wheel = false, bool door = false); //

///////////////////////功能3：渲染单视图部分///////////////////////////////
//////////////single view ///////////////////
typedef struct {
  int x;
  int y;
  int w;
  int h;
} ROIParameter;
typedef struct {
  Patch *singleview;
  struct SideView {
    float color[4] = {1.0, 1.0, 0.0, 1.0};
    Patch *sideView;
  } side;
  struct WideView {
    float color[4] = {1.0, 0.0, 0.0, 1.0};
    Patch *wideAngleView;
  } wide;

  Patch *lineView;
  GLuint buffer;
  GLuint prog;                // shader program
  GLuint prog_line;           // line shader program
  GLuint prog_line_textsmall; // shader program
  ROIParameter ROIPara;
  bool isDMA = false; // false:not support DMA;true:support DMA;
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
  int view;

} avm_single_type_t;

int xcb_avmlib_single_Init(avm_single_type_t *avm_single_type);
int xcb_avmlib_single_Exit(avm_single_type_t *avm_single_type);
/*

avm_single_view_type_t *avm_single_view:singleview variable
const char *default_filename:the default file's name of stitching(
    if "filename" cannot access,"default_filename" will be used )
return value: -1:fail,0:succeed
*/
int xcb_avmlib_single_Gen_VBO(const char *filename,
                              const char *default_filename,
                              avm_single_type_t *avm_single_type);
/*
int type:type=0:sideview front,type=1:sideview rear ,type=2: sideview up
*/
int xcb_avmlib_sideView_Gen_VBO(const char *filename,
                                const char *default_filename,
                                avm_single_type_t *avm_single_type, int type);
int xcb_avmlib_wideAngleView_Gen_VBO(const char *filename,
                                     const char *default_filename,
                                     avm_single_type_t *avm_single_type);

////////////////////////////////////////////////////////////////////////////////////////
/*Render single view
Note:use the same texture buffer id with stitching texture_id
*/
int xcb_avmlib_singleView_Render(avm_single_type_t *avm_single_type,
                                 ViewParameter &viewParameter,
                                 avm_camera_type_t *camera,
                                 GLuint *g_texture_id);

/*Render side view
int type:type=0:sideview front,type=1:sideview rear ,type=2: sideview up
Note:use the same texture buffer id with stitching texture_id
*/
int xcb_avmlib_sideView_Render(avm_single_type_t *avm_single_type,
                               ViewParameter &viewParameter,
                               avm_camera_type_t *camera, GLuint *g_texture_id,
                               int type, bool left = true, bool right = true);
/*Render wideangle view
int type:type=0:wideangle view front,type=1:wideangle view rear
Note:use the same texture buffer id with stitching texture_id
*/
int xcb_avmlib_wideAngleView_Render(avm_single_type_t *avm_single_type,
                                    ViewParameter &viewParameter,
                                    avm_camera_type_t *camera,
                                    GLuint *g_texture_id, int type);

////////////////////added by zliu 20200315 for adjusting virtual camera in real
/// time

//////////////added by zliu 20200315 for adjusting virtual camera in real time
/////////////功能3.1：实时更新相关功能参数，并显示
struct VirCamPara {
  float fCamPosition[3];
  float fCamFrontDirect[3];
  float fCamUpDirect[3];
  float fCamZoom;
  float fCamfocal;
  int nOutputImgH;
  int nOutputImgW;
};
typedef struct _SideViewVirCamPara {
  ////////////////////在原始1920*1280的图像上，截取的区域，越大则视野范围越大
  int nStartPixelX[2];
  int nEndPixelX[2];
  int nStartPixelY[2];
  int nEndPixelY[2];

  /////////////////////////虚拟视角相关参数，注意左右sideview是对称的，给的是左侧的虚拟视角参数
  float fCamPosition[3];
  float fCamFrontDirect[3];
  float fCamUpDirect[3];
  float fCamZoom;
  float fCamfocal;

  int nOutputImgH;
  int nOutputImgW;

  ///////////////////////警示线的物理位置
  float fLine[8];
} SideViewVirCamPara;

int xcb_avmlib_singleView_UpdateVirtualCamera(
    struct VirCamPara *pVCInfor, int nCameraIndex,
    avm_single_type_t *avm_single_type);
int xcb_avmlib_singleView_SavingVirtualPara(const char *filename,
                                            avm_single_type_t *avm_single_type);
int xcb_avmlib_singleView_ObtainVirtualCamera(
    struct VirCamPara *pVCInfor, int nCameraIndex,
    avm_single_type_t *avm_single_type);
int xcb_avmlib_singleView_ResetVirtualCamera(
    int nCameraIndex, avm_single_type_t *avm_single_type);

int xcb_avmlib_sideView_UpdateVirtualCamera(
    struct _SideViewVirCamPara *pVCInfor, int nType, float *fInputLine,
    avm_single_type_t *avm_single_type);
int xcb_avmlib_sideView_SavingVirtualPara(const char *filename,
                                          avm_single_type_t *avm_single_type,
                                          int type);
int xcb_avmlib_sideView_ObtainVirtualCamera(
    struct _SideViewVirCamPara *pVCInfor, int nType,
    avm_single_type_t *avm_single_type);
int xcb_avmlib_sideView_ResetVirtualCamera(int nType,
                                           avm_single_type_t *avm_single_type);

int xcb_avmlib_wideAngleView_UpdateVirtualCamera(
    struct VirCamPara *pVCInfor, int nCameraIndex,
    avm_single_type_t *avm_single_type);
int xcb_avmlib_wideAngleView_SavingVirtualPara(
    const char *filename, avm_single_type_t *avm_single_type);
int xcb_avmlib_wideAngleView_ObtainVirtualCamera(
    struct VirCamPara *pVCInfor, int nCameraIndex,
    avm_single_type_t *avm_single_type);
int xcb_avmlib_wideAngleView_ResetVirtualCamera(
    int nCameraIndex, avm_single_type_t *avm_single_type);

//////////////////////////////在单视图上画轨迹线，该功能主要用于D357项目中///////////////////////
///////////////////前后单视图可能都会贴轨迹线图，但是他们目前是共用vbo
/// buffer，所以在进行单视图前后切换时需要更新VBO
/// BUFFER的内容，更新时需要注意ROI的设置参数不一样。
///在生成轨迹线贴图的方式是，依据工具生成轨迹线点之后，给美工做出轨迹线贴图（轨迹线贴图的宽高和单视图原始图片的宽高一致），为了提高效率设置轨迹线在原图中的roi区域，该ROI区域对应参数为ROIParameter
/////////////功能3.2：在单视图上更新轨迹线贴图，目前只有D357使用
///////////创建vbo
int xcb_avmlib_singleview_DynamicLineText_Gen_VBO(
    avm_single_type_t *avm_single_type);
///////////更新vbo内容，主要是前后切换时
int xcb_avmlib_singleview_DynamicLineText_Update_VBO(
    avm_single_type_t *avm_single_type);
///////////更新轨迹线对应的图片，每个角度贴图不一样
int xcb_avmlib_Update_SingleView_Png_line(avm_single_type_t *avm_single_handle,
                                          int layer_index,
                                          png_texture_t *png_texture,
                                          int png_num);
///////////渲染轨迹线
int xcb_avmlib_single_DynamicLineText_Render(avm_single_type_t *avm_single_type,
                                             ViewParameter &viewParameter,
                                             avm_camera_type_t *camera,
                                             GLuint *g_texture_id);

///////////////////////功能4：轨迹线功能////////////////////////////////////////////
///////////////lines///////////////////
/////////注意，单位毫米/////////
typedef struct {
  int width;    //线图片的宽
  int heigth;   //线图片的高
  int channels; //线图片的通道数
  unsigned char
      *pixel; //指针指向使用的纹理图片的像素值，默认四通道的纹理图片，即RGBA
} Lines_Texture;

typedef struct {
  int type;        // type=0:前进轨迹线,type=1:后退轨迹线
  float fMaxAngle; //轮胎摆动的最大角度，注意此值为正值
  float fAngle; //轮胎摆动的角度，大于0向右，小于0向左，等于0，未摆动
  int iLineLength;   //  设置轨迹线的长度
  int thick;         //设置单视图线的粗细
  int pointnum;      //设置鸟瞰图拟点的数量
  Lines_Texture tex; //指向使用图片的纹理
  GLuint texture_id; //俯视图和单视图的动态轨迹线使用一种颜色
  struct Single_Line {
    Patch *single_line;  //初始化内部开辟资源，退出释放
    ViewParameter *view; //指针指向单视图的虚拟相机
    float *color; //指向设置单视图警戒线的颜色如：color[4]={1.0,1.0,0.0,1.0}
    bool isResived = false; //标识是否校正，true为校正false为未校正
  } single;
} Line_dynamic_single;
typedef struct {
  int type;        // type=0:前进轨迹线,type=1:后退轨迹线
  float fMaxAngle; //轮胎摆动的最大角度，注意此值为正值
  float fAngle; //轮胎摆动的角度，大于0向右，小于0向左，等于0，未摆动
  int iLineLength;   //  设置轨迹线的长度
  int thick;         //设置单视图线的粗细
  int pointnum;      //设置鸟瞰图拟点的数量
  Lines_Texture tex; //指向使用图片的纹理
  GLuint texture_id; //俯视图和单视图的动态轨迹线使用一种颜色
  struct Bird_Line {
    Patch *bird_line;    //初始化内部开辟资源，退出释放
    ViewParameter *view; //指针指向鸟瞰图图的虚拟相机
  } bird;
} Line_dynamic_bird;
typedef struct xcb_avmlib {
  int type;            // type=0:前进轨迹线,type=1:后退轨迹线
  int thick;           //设置静态轨迹线的粗细
  int scale_length;    //设置刻度尺的长度
  int stepDistance[3]; //分别表示刻度尺距车头或车尾线的距离
  GLuint *texture_id; //存储静态线使用纹理的id，初始化开辟资源，
  Lines_Texture tex[3]; //指向使用图片的纹理,刻度尺的和纹理一一对应
  ViewParameter *view; //指针指向鸟瞰图图的虚拟相机
  Patch *lines;        //初始化内部开辟资源，退出释放
  bool isResived;      //标识是否校正，true为校正false为未校正
} Line_static;

typedef struct {
  int thick_Level;    //设置水平线的宽度
  int thick_Vertical; //设置垂直线的宽度
  float LevelPos_left
      [4]; //设置左侧水平线左右起点和终点的位置,坐标设置按照（xyxy）的顺序
  float VerticalPos_left
      [4]; //设置左侧垂直线左右起点和终点的位置,坐标设置按照（xyxy）的顺序
  float LevelPos_Right
      [4]; //设置右侧水平线左右起点和终点的位置,坐标设置按照（xyxy）的顺序
  float VerticalPos_Right
      [4]; //设置右侧垂直线左右起点和终点的位置,坐标设置按照（xyxy）的顺序
  ViewParameter *view; //指针指向单视图的虚拟相机
  Lines_Texture tex;   //设置线的颜色，注意使用纹理图片
  bool
      flag; //设置是否计算辅助线的点和是否显示辅助线，flag=true时，计算辅助线的点，和显示辅助线
  Patch *lines; //初始化开辟资源，退出释放资源
} Lines_Side;

typedef struct {
  int iCarLength;            //设置车长
  int iLineDistance;         //设施两侧线之间的距离
  GLuint prog;               // shader program
  Line_static *lines_static; //初始化内部开辟资源，退出释放
} avm_line_static_type_t;
typedef struct {
  int iCarLength;                       //设置车长
  int iLineDistance;                    //设施两侧线之间的距离
  int iFrontWheelDis;                   //车前轮轴心距车头的距离
  int iRearWheelDis;                    //车后轮轴心距车尾的距离
  int iWheelBase;                       //前后车轮轴心之间的间距；
  GLuint prog;                          // shader program
  Line_dynamic_bird *line_dynamic_bird; //初始化内部开辟资源，退出释放
  bool bDemoorNot[4]; //每三根线为一组，对应的标记为true，则显示，否则不显示
  float fratio[4];     //一组三根线，每根线对应的透明度
  int nSingleLineMode; // 0: 每根线用单独贴图表示；1:
                       // 前或者后的两条线用一张贴图表示
} avm_line_dynamic_bird_type_t;
typedef struct {
  int iCarLength;     //设置车长
  int iLineDistance;  //设施两侧线之间的距离
  int iFrontWheelDis; //车前轮轴心距车头的距离
  int iRearWheelDis;  //车后轮轴心距车尾的距离
  int iWheelBase;     //前后车轮轴心之间的间距；
  GLuint prog;        // shader program
  Line_dynamic_single *line_dynamic_single; //初始化内部开辟资源，退出释放
  bool bDemoorNot[4]; //每三根线为一组，对应的标记为true，则显示，否则不显示
  float fratio[4];     //一组三根线，每根线对应的透明度
  int nSingleLineMode; // 0: 每根线用单独贴图表示；1:
                       // 前或者后的两条线用一张贴图表示
} avm_line_dynamic_single_type_t;

typedef struct {
  int iCarLength;         //设置车长
  int iLineDistance;      //设施两侧线之间的距离
  int iFrontWheelDis;     //车前轮轴心距车头的距离
  int iRearWheelDis;      //车后轮轴心距车尾的距离
  int iWheelBase;         //前后车轮轴心之间的间距；
  GLuint prog;            // shader program
  Lines_Side *lines_side; //初始化内部开辟资源，退出释放
} avm_line_Side_type_t;

// statci line
int xcb_avm_line_static_Init(avm_line_static_type_t *avm_lines_type);
int xcb_avm_line_static_Gen_VBO(avm_line_static_type_t *avm_lines_type,
                                avm_single_type_t *avm_single_type);
int xcb_avm_line_static_Exit(avm_line_static_type_t *avm_lines_type);
int xcb_avm_line_static_Update(avm_line_static_type_t *avm_lines_type,
                               avm_single_type_t *avm_single_type);
int xcb_avm_line_static_Render(avm_line_static_type_t *avm_lines_type,
                               float fAlpha);

// dynamic line for bird view
int xcb_avm_line_dynamic_bird_Init(
    avm_line_dynamic_bird_type_t *avm_lines_type);
int xcb_avm_line_dynamic_bird_Gen_VBO(
    avm_line_dynamic_bird_type_t *avm_lines_type,
    avm_single_type_t *avm_single_type);
int xcb_avm_line_dynamic_bird_Exit(
    avm_line_dynamic_bird_type_t *avm_lines_type);
int xcb_avm_line_dynamic_bird_Update(
    avm_line_dynamic_bird_type_t *avm_lines_type,
    avm_single_type_t *avm_single_type, avm_car_type_t *avm_car_Handle);
int xcb_avm_line_dynamic_bird_Render(
    avm_line_dynamic_bird_type_t *avm_lines_type, bool bAlarmLine = true);

// dynamic line for single view
int xcb_avm_line_dynamic_single_Init(
    avm_line_dynamic_single_type_t *avm_lines_type);
int xcb_avm_line_dynamic_single_Gen_VBO(
    avm_line_dynamic_single_type_t *avm_lines_type,
    avm_single_type_t *avm_single_type);
int xcb_avm_line_dynamic_single_Exit(
    avm_line_dynamic_single_type_t *avm_lines_type);
int xcb_avm_line_dynamic_single_Update(
    avm_line_dynamic_single_type_t *avm_lines_type,
    avm_single_type_t *avm_single_type, avm_car_type_t *avm_car_Handle);
int xcb_avm_line_dynamic_single_Render(
    avm_line_dynamic_single_type_t *avm_lines_type);

///////////////for side line/////////////////
int xcb_avm_side_lines_Init(avm_line_Side_type_t *avm_line_type);
int xcb_avm_side_lines_Gen_VBO(avm_line_Side_type_t *avm_lines_type);
int xcb_avm_side_lines_Update(avm_line_Side_type_t *avm_line_type,
                              avm_single_type_t *avm_single_type);
int xcb_avm_side_lines_Exit(avm_line_Side_type_t *avm_line_type);
int xcb_avm_side_lines_Render(avm_line_Side_type_t *avm_line_type, bool left,
                              bool right);

//////////////////////point translation, it is an example for shi kangli, not
/// output to others. nChannel: 0, back; 1, Left; 2, Front; 3,
/// Right///////////////
/////////////2D original image translation
int xcb_avmlib_2DOrignLine_Translation(float *fPointsIn, int pointnum,
                                       float *fPointsOut, int nChannel,
                                       avm_single_type_t *avm_single_type);
/////////////2D revised image translation
int xcb_avmlib_2DRevisedLine_Translation(float *fPointsIn, int pointnum,
                                         float *fPointsOut, int nChannel,
                                         avm_single_type_t *avm_single_type);

/////////////功能5：雷达功能
#define MAX_RADAR_REGION_CNT 1
struct Radar {
  unint_8 radarId; //雷达id号
  bool targetState
      [MAX_RADAR_REGION_CNT]; // true:表示有障碍物，false:表示无障碍物
  double targetDistance[MAX_RADAR_REGION_CNT]; //障碍物的距离，以雷达为中心
  unint_8 targetWarnLevel
      [MAX_RADAR_REGION_CNT]; //障碍物显示的颜色，0：红色，1：黄色，2：绿色
  double radarPos[2]; //以车模为中心雷达安装的位置
  int space; //扇形大小，即外环与内环的半径差，单位为毫米
  int region_cnt;    //每个雷达的共多少目标区域, 1
  double startAngle; //以雷达为中心的起始角度
  double endAngle;   //以雷达为中心的终止角度
  int posType; //表示雷达安装在车的那个边，0:车前边，1:车左边，2:车后边，3:车右边
  int high;    //扇形高度，单位为毫米
  double startPoint[2]; //以雷达为中心的扇面起始点
  double endPoint[2];   //以雷达为中心的扇面终止点
  float fALPHA;
};
typedef struct {
  int iPosX;      // start position of car's X coordinate, left top
  int iPosY;      // start position of car's Y coordinate, left top
  int iCarWidth;  // 2d car's width
  int iCarLength; // 2d car's length
  GLuint prog;    // shader program
} avm_radar_type_t;

void xcb_avmlib_InitRadar(avm_radar_type_t *avm_radar_type, int radar_cnt = 4,
                          int region_cnt = 1);

/*@param传入雷达信息，更新雷达信息
 * input:
 * avm_radar_type_t avm_radar_type:
 * int radar_cnt：雷达的数量
 * int region_cnt ：扇区数
 */

/*@param传入雷达信息，更新雷达信息
 * input:
 * Radar *radar:雷达信息
 * int radar_cnt：雷达的数量
 */
void xcb_avmlib_updateRadar_Sector(struct Radar *radar, int cnt,
                                   avm_single_type_t *avm_single_type);
/*@param渲染雷达
 * struct Radar *radar ：雷达
 * int cnt=4：雷达数量，默认为4个
 * */
void xcb_avmlib_RenderRadar_Sector_2D(avm_radar_type_t *avm_car_type,
                                      ViewParameter viewParameter,
                                      struct Radar *radar, int cnt);

void xcb_avmlib_RenderRadar_Sector_3D(avm_radar_type_t *avm_car_type,
                                      ViewParameter viewParameter,
                                      struct Radar *radar, int cnt);
/*
view=1~8;当前singleview所显示的画面内容
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
void xcb_avmlib_RenderRadar_Sector_SingleView(avm_radar_type_t *avm_car_type,
                                              ViewParameter viewParameter,
                                              struct Radar *radar, int cnt,
                                              int view);

void xcb_avmlib_Radar_exit(int radar_cnt, int region_cnt);
//////////////用于贴雷达出错时的贴图
typedef struct {
  Patch_Lines *radar_break_patch;
  int texture_width;    // bottom texture's width
  int texture_height;   // bottom texture's height
  int texture_channels; //底部纹理的通道数
  unsigned char *pixel; //指针指向存储底部纹理的像素地址
  float ftopx;          //需要贴图的物理位置的左上角
  float ftopy;          //需要贴图的物理位置的右上角
  float fwidth;         //物理位置的宽度
  float fheight;        //物理位置的高度
  GLuint prog;          // shader program
} avm_radar_break_t;
int xcb_avmlib_InitRadarBreak(avm_radar_break_t *avm_radar_break);
void xcb_avmlib_RadarBreak_exit(avm_radar_break_t *avm_radar_break);
void xcb_avmlib_RenderRadarBreak(avm_radar_break_t *avm_radar_break,
                                 ViewParameter viewParameter);

//////////////////////功能6：ADAS的辅助画线打点功能
/////////////// ADAS Auxiliary display//////////////
typedef struct {
  int id;
  int vertex_number;   // 模型顶点数
  int tri_face_number; //三角片元总数
  GLuint *buffers;     // vertex buffer
                   // id（顶点坐标+4*纹理坐标+4*混合权重+三角片元索引）
  GLuint *texture_id;
  void *pData; // added by zliu for update virtual camera 20200315
} Patch_adas;

typedef struct {
  Patch_adas *adas;
  GLuint prog; // shader program
} adas_auxiliary_type_t;
int xcb_avm_adas_Init(adas_auxiliary_type_t *adas_auxiliary);
/*
adas_auxiliary_type_t *adas_auxiliary//初始化的句柄
int points_num,//输入点的个数
float *points,//指针指向输入的点的数组
int type//表示是矩形框还是线，type=0，表示矩形框，type=1，表示线
*/
int xcb_avm_adas_Update(adas_auxiliary_type_t *adas_auxiliary, int points_num,
                        float *points, int type);
///更新线的
/*
adas_auxiliary_type_t *adas_auxiliary,
int points_num, //点的个数
float *points,  //指针指向的数组
int type//type=0,//表示更新左侧的线，type=1表示更新右侧的线；
*/
int xcb_avm_adas_Lines_Update(adas_auxiliary_type_t *adas_auxiliary,
                              int points_num, float *points, int thick,
                              int type);
/*
adas_auxiliary_type_t *adas_auxiliary//初始化的句柄
ViewParameter *view//指针指向使用的虚拟参数的变量
float *color//指向使用的颜色颜色数组如：（1.0，0.0，0.0，1.0）
int type//表示是矩形框还是线，type=0，表示矩形框，type=1，表示线
float fLineWidth, 线的宽度
*/
void xcb_avm_adas_Render(adas_auxiliary_type_t *adas_auxiliary,
                         ViewParameter *view, float *color, int type, float fLineWidth = 5.0);
/*
adas_auxiliary_type_t *adas_auxiliary//初始化的句柄
ViewParameter *view//指针指向使用的虚拟参数的变量
float *color//指向使用的颜色颜色数组如：（1.0，0.0，0.0，1.0）
int type//表示是矩形框还是线，type=0，表示左侧的线，type=1，表示右侧的线
*/
void xcb_avm_adas_Lines_Render(adas_auxiliary_type_t *adas_auxiliary,
                               ViewParameter *view, float *color, int type);

// Round_Dot
typedef struct {
  GLuint prog; // shader program
  GLuint dot;  // the dot buffer id
  float
      color[4]; // the dot's color ,0~3:r,g,b,a,Note:the range of r/g/b/a is 0~1
  int point_number; // the number of dots
  float point_size; // the size of dot
} avm_dot_type_t;
/*
Initialization avm_dot_type_t
*/
int xcb_avmlib_Draw_Round_Dot_Init(avm_dot_type_t *avm_dot_type);
/*update points
input:
avm_dot_type_t *avm_dot_type//dot Handle
float *points//the dot position coordnate,Note:one point includes x, y, and z
coornate int point_num//the number of points
*/
int xcb_avmlib_Draw_Round_Dot_Update(avm_dot_type_t *avm_dot_type,
                                     float *points, int point_num);
/*
Render interface
avm_dot_type_t *avm_dot_type//dot Handle
ViewParameter viewParameter//the view parameter
*/

int xcb_avmlib_Draw_Round_Dot_Render(avm_dot_type_t *avm_dot_type,
                                     ViewParameter viewParameter);

void xcb_avmlib_Draw_Round_Dot_Exit(avm_dot_type_t *avm_dot_type);

/////////////////////////////功能8：透明底盘，FBO版本/////////////////////////////
typedef struct {
  int vertex_number;   // 模型顶点数
  int tri_face_number; //三角片元总数
  float *pPosition;
  float *pBlindPos;
  float *pPosNormalized;
  float *pBlindPosNormalized;
  float *pCord;
  float *pCordRun;
  float *pCorSubWin;
  float *pStitchingPos[4];
  float *pWeightStiting[4];
  int pStitchingvertexnum[4];
  float *
      pOutPosition; ////////有运动估计算法，根据pPosition中的数据更新pOutPosition位置
  float *pWeightforblending;
  GLuint fbo_Framebuffer;
  GLuint fbo_Texturebuffer[2];
  GLuint buffers[10 + 4 + 2 + 1]; // vertex buffer

} Patch_transchassis_fbo;
typedef struct {
  Patch_transchassis_fbo *fbo_transchassis_patch;
  GLuint prog;             // shader program，纹理贴图shader
  GLuint Stitchingprog;    // shader program，环视拼接贴图shader
  GLuint prog_viewpara;    //
  GLuint prog_viewparawei; //
  GLuint prog_wei;         // shader program，纹理贴图shader
  // GLuint prog_viewpara;  //
  float patchsize_width;  //整个物理坐标系的宽度，单位mm
  float patchsize_height; //整个物理坐标系的高度，单位mm
  int window_width;       //窗口的宽度，或者说texture的宽度
  int window_height;      //窗口的高度，或者说texture的高度
  int nPatch_Bin_X;       //用于构建底纹x方向上网格点数量
  int nPatch_Bin_Y;       //用于构建底纹y方向上网格点数量
  int nFirstRender;       //是否是第一次渲染
  int nChangeIndexornot;  //是否进行切换渲染texuture操作
  int ISDMA;
  void *fboObject1;
  void *fboObject2;
  int nRenderIndex; // texture buffer中用于渲染的标记texture的index
  float fBlindAreaWidth;
  float fBlindAreaHeight;
  float fBlindcx;
  float fBlindcy;
  int blending_bin_x; //小于nPatch_Bin_X/2
  int blending_bin_y; //小于nPatch_Bin_Y/2
} avm_fbo_transchassis_type_t;
//////////////////fbo版本透明底盘初始化////////////////////////////////////
int xcb_avm_fbo_transchassis_Init(
    avm_fbo_transchassis_type_t *avm_fbo_transchassis,
    avm_stitching_type_t *avm_stitching_type);
int xcb_avm_fbo_transchassis_Exit(
    avm_fbo_transchassis_type_t *avm_fbo_transchassis);
/////////////////////返回渲染后的texture
/// id///////////////////////////////////////
int xcb_avm_fbo_transchassis_Update(
    avm_fbo_transchassis_type_t *avm_fbo_transchassis);
GLuint xcb_avm_fbo_transchassis_Render(
    avm_fbo_transchassis_type_t *avm_fbo_transchassis,
    avm_stitching_type_t *avm_stitching_type, ViewParameter &viewParameter,
    GLuint *g_texture_id);
int xcb_avm_fbo_transchassis_Texture_Renderwithviewpara2D(
    avm_fbo_transchassis_type_t *avm_fbo_transchassis,
    ViewParameter &viewParameter, GLuint *g_texture_id);
int xcb_avm_fbo_transchassis_Texture_Renderwithviewpara3D(
    avm_fbo_transchassis_type_t *avm_fbo_transchassis,
    ViewParameter &viewParameter, GLuint *g_texture_id);
/////////////////////用于测试GPU的纹理/////////////////////////////////////

////////////////////功能9：调试用于原始画面和视频显示功能
//////////////////////////////////////////////////////////////////////////
////////////////////full screen//////////////////
typedef struct {
  int win_x;
  int win_y;
  int width;
  int height;
  Patch *screen;
  GLuint prog;
  bool isDMA = false;
} avm_screen_type_t;
int xcb_avmlib_screen_Init(avm_screen_type_t *avm_screen_type);
/*
type=1,
*/
int xcb_avmlib_screen_Gen_VBO(avm_screen_type_t *avm_screen_type);
int xcb_avmlib_full_screen_Render(avm_screen_type_t *avm_screen_type,
                                  GLuint *g_textureid, int index);
int xcb_avmlib_four_screen_Render(avm_screen_type_t *avm_screen_type,
                                  GLuint *g_textureid);
int xcb_avmlib_six_screen_Render(avm_screen_type_t *avm_screen_type,
                                 GLuint *g_textureid);

#endif