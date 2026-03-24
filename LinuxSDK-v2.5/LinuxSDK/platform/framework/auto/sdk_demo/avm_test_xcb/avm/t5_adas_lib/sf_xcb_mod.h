#ifndef __SF_XCB_MOD_H__
#define __SF_XCB_MOD_H__

/************************************************************************/
/* MOD declaration                                                     */
/************************************************************************/

/** Which side is the MOD warning from (MOD报警来自于哪一侧的类型)*/
typedef enum sf_xcb_mod_side_s
{
  SF_XCB_MOD_Back = 0,
  SF_XCB_MOD_Left = 1,
  SF_XCB_MOD_Front = 2,
  SF_XCB_MOD_Right = 3
} sf_xcb_mod_side_t;

/** the gear of the car (输入车的挡位状态)
**Parking_gear停车档
**None_gear空挡
**Reverse_gear倒车档
**Drive_gear行驶档
*/
typedef enum sf_xcb_mod_car_gear_s
{
  Parking_gear = 0,
  None_gear = 1,
  Reverse_gear = 2,
  Drive_gear = 3
} sf_xcb_mod_car_gear_state_t;

/** Type of MOD detect obj output (MOD的输出类型)*/
typedef struct Obj_Rect_Position
{
  int x; /**< MOD warning obj rect position(left up x ) (目标框左上X坐标) */
  int y; /**< MOD warning obj rect position(left up y ) (目标框左上Y坐标) */
  int w; /**< MOD warning obj rect position(rect width ) (目标框的宽度) */
  int h; /**< MOD warning obj rect position(rect height ) (目标框的高度) */
  double lux;
  double luy;
  double rux;
  double ruy;
  double rdx;
  double rdy;
  double ldx;
  double ldy;
  float world_x = 0.0; /**< MOD warning obj ground position (目标世界坐标的X方向距离) */
  float world_y = 0.0; /**< MOD warning obj ground position (目标世界坐标的Y方向距离) */
} obj_rect_position_t;

/** Type of MOD output (MOD的输出类型)*/
typedef struct sf_xcb_mod_output_s
{
  sf_xcb_mod_side_t
      idxSide;              /**< Which side is the MOD warning from (报警来自于哪一侧) */
  int nObjNum;              /**< The number of detected objects (检测到的物体数目) */
  obj_rect_position_t *obj; /**< position of all objects (所有目标的中点横坐标) */
} sf_xcb_mod_output_t;

/************************************************************************/
/* MOD APIs                                                            */
/************************************************************************/

/** Initialize instance of MOD (MOD实例初始化函数)
 *
 *  @param [in] dist_horizontal: Detection distance in horizontal direction
 *                              (水平检测距离，未用)
 *  @param [in] dist_vertical: Detection distance in vertical direction
 *                              (垂直检测距离，未用)
 *	@param [in] width: Width of input gray image (输入图像的宽)
 *  @param [in] height: Height of input gray image (输入图像的高)
 *
 *  @return Pointer to the created MOD module
 *          (返回：MOD模块的指针，失败返回NULL)
 */
void *sf_xcb_mod_init(int dist_horizontal, int dist_vertical, int width,
                      int height, const char *pCalibOutPath, const char *pCalibOutPath_Default,const char *pAuthFilePath);

/** Deallocate MOD and free resources (MOD相关资源的释放)
 *
 *  @param [in] ppMOD: Address of pointer of MOD module (MOD模块指针的地址)
 *
 *  @return Status. None zero if error (MOD释放状态，0-没有错误，非0-释放失败)
 */
int sf_xcb_mod_release(void **ppMOD);

/** Process new video frame and get MOD result
 * (对当前帧图像进行处理并返回MOD结果)
 *
 *  @param [in] pMOD: Pointer to the created MOD module (MOD模块指针)
 *  @param [in] gray_back: gray frame of back-side camera (后侧摄像头的灰度图)
 *  @param [in] gray_left: gray frame of left-side camera (左侧摄像头的灰度图)
 *  @param [in] gray_front: gray frame of front-side camera (前侧摄像头的灰度图)
 *  @param [in] gray_right: gray frame of right-side camera (右侧摄像头的灰度图)
 *  @param [in] fSpeed: Speed of four wheels, km/h (当前车速，单位km/h)
 *  @param [in] car_gear_state:which gear the car is used(车当前档位信息)
 *  @param [in] fAngle: Rotation angle of the steering wheel, degree
 *                      (方向盘偏转角，单位度数，暂未使用)
 *  @param [out] output: Returned pointer to MOD output. Must not be freed!
 *                      (返回的MOD结果，不能被外部释放)
 *
 *  @return void
 */
void sf_xcb_mod_process(void *pMOD, unsigned char *gray_back,
                        unsigned char *gray_left, unsigned char *gray_front,
                        unsigned char *gray_right, float fSpeed,
                        sf_xcb_mod_car_gear_state_t car_gear_state,
                        float fAngle, sf_xcb_mod_output_t **output);

//void getObj_info(void *pMOD, sf_xcb_mod_output_t **output);
#endif /*__SF_XCB_MOD_H__*/
