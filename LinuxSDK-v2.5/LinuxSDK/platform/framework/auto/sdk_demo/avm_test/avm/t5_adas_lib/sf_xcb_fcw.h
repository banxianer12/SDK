#ifndef __SF_XCB_FCW_H__
#define __SF_XCB_FCW_H__

/************************************************************************/
/* FCW declaration                                                     */
/************************************************************************/

/** Type of FCW detect obj output (FCW的输出类型)*/
typedef struct FCW_Obj_Rect_Position
{
	int x;			/**< FCW warning obj rect position(left up x ) (目标框左上X坐标) */
	int y;			/**< FCW warning obj rect position(left up y ) (目标框左上Y坐标) */
	int w;			/**< FCW warning obj rect position(rect width ) (目标框的宽度) */
	int h;			/**< FCW warning obj rect position(rect height ) (目标框的高度) */
	int label;		/**< FCW warning obj label (目标编号) */
	float speed;	/**< FCW warning obj move speed(目标运动速度) */
	float distance; /**< FCW warning obj distance to self car (目标距自身车头距离) */
} fcw_obj_rect_position_t;

/** Type of fcw output (FCW的输出类型)*/
typedef struct sf_xcb_fcw_output_s
{
	int nObjNum;			  /**< The number of detected objects (检测到的物体数目) */
	fcw_obj_rect_position_t *obj; /**< position of all objects (所有目标的中点横坐标) */
} sf_xcb_fcw_output_t;

/************************************************************************/
/* FCW APIs                                                            */
/************************************************************************/

/** Initialize instance of FCW (FCW实例初始化函数)
 *
 *  @param [in] dist_horizontal: Detection distance in horizontal direction
 *                              (水平检测距离，未用)
 *  @param [in] dist_vertical: Detection distance in vertical direction
 *                              (垂直检测距离，未用)
 *	@param [in] width: Width of input gray image (输入图像的宽)
 *  @param [in] height: Height of input gray image (输入图像的高)
 *
 *  @return Pointer to the created FCW FCWule
 *          (返回：FCW模块的指针，失败返回NULL)
 */
void *sf_xcb_fcw_init(int dist_horizontal, int dist_vertical, int width,
					  int height, const char *pCalibOutPath, const char *pCalibOutPath_Default,const char *pAuthFilePath);

/** Deallocate FCW and free resources (FCW相关资源的释放)
 *
 *  @param [in] ppFCW: Address of pointer of FCW FCWule (FCW模块指针的地址)
 *
 *  @return Status. None zero if error (FCW释放状态，0-没有错误，非0-释放失败)
 */
int sf_xcb_fcw_release(void **ppfcw);

/** Process new video frame and get FCW result
 * (对当前帧图像进行处理并返回FCW结果)
 *
 *  @param [in] pFCW: Pointer to the created FCW FCWule (FCW模块指针)
 *  @param [in] gray_back: gray frame of back-side camera (后侧摄像头的灰度图)
 *  @param [in] gray_left: gray frame of left-side camera (左侧摄像头的灰度图)
 *  @param [in] gray_front: gray frame of front-side camera (前侧摄像头的灰度图)
 *  @param [in] gray_right: gray frame of right-side camera (右侧摄像头的灰度图)
 *  @param [in] fSpeed: Speed of four wheels, km/h (当前车速，单位km/h)
 *  @param [in] car_gear_state:which gear the car is used(车当前档位信息)
 *  @param [in] fAngle: Rotation angle of the steering wheel, degree
 *                      (方向盘偏转角，单位度数，暂未使用)
 *  @param [out] output: Returned pointer to FCW output. Must not be freed!
 *                      (返回的FCW结果，不能被外部释放)
 *
 *  @return void
 */
void sf_xcb_fcw_process(void *pfcw, unsigned char *gray_front, float fSpeed);

//get front detect obj
void sf_xcb_fcw_get_detect_obj(void *pfcw, sf_xcb_fcw_output_t **output);
#endif /*__SF_XCB_FCW_H__*/
