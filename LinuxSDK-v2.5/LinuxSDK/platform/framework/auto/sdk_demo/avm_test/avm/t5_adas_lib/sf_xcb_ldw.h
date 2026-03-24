#ifndef __SF_XCB_LDW_H__
#define __SF_XCB_LDW_H__

/************************************************************************/
/* LDW declaration                                                     */
/************************************************************************/
/**car move side**/
enum car_move_side
{
	CAR_NO_SHIFT = 0,
	CAR_SHIFT_TO_LEFT = 1,
	CAR_SHIFT_TO_RIGHT = 2
};
/** Parameters of ldw */
typedef struct sf_xcb_ldw_params_s
{
  float fStSpeed;  /**< Standby speed (km/h)*/
  float fActSpeed; /**< Activated speed (km/h)*/
  int nCarWidth;   /**< Width of car between two frontal/rear tire outside(cm)*/
  int nWheelbase;  /**< Wheelbase(cm)*/
} sf_xcb_ldw_params_t;
//line4int -- line is described by 2 points(x0,y0,x1,y1)
typedef struct _line4int
{
  int x0;
  int y0;
  int x1;
  int y1;
} line4int;

typedef struct _ldw_output_line_with_color
{
  line4int line; //left line=line[0]up_point=(x0,y0) down_point=(x1,y1)[1]
  int R;         //color RGB R
  int G;         //color RGB R
  int B;         //color RGB R
} ldw_output_line_with_color_t;
/** The color of lane lines */
typedef enum sf_xcb_line_color_s
{
  SF_XCB_LDW_LINE_WHITE = 0,
  SF_XCB_LDW_LINE_YELLOW = 1
} sf_xcb_line_color_t;

/** The type of lane lines */
typedef enum sf_xcb_line_type_s
{
  SF_XCB_LDW_LINE_SINGLE_DOTTED = 0,
  SF_XCB_LDW_LINE_SINGLE_SOLID = 1,
  SF_XCB_LDW_LINE_DOUBLE_DOTTED = 2,
  SF_XCB_LDW_LINE_DOUBLE_SOLID = 3
} sf_xcb_line_type_t;

/** Description of a single detected road line */
typedef struct sf_xcb_ldw_road_mark_line_s
{
  int isDetected;                /**< 1 if detected, 0 if not detected */
  sf_xcb_line_type_t lineType;   /**< The type of lane lines */
  sf_xcb_line_color_t lineColor; /**< The color of lane lines */
  float fDistToTire;             /**< Distance from lane line to tire outside of car */
} sf_xcb_ldw_road_mark_line_t;

/** Couple of road lines */
typedef struct sf_xcb_ldw_output_s
{
  sf_xcb_ldw_road_mark_line_t leftLine;
  sf_xcb_ldw_road_mark_line_t rightLine;
  ldw_output_line_with_color_t leftcolorline;
  ldw_output_line_with_color_t rightcolorline;
  float fDepartSpeed; /**< Departed speed(m/s), >0 departed to left, <0 to right
                       */
  float fTTLC;        /**< Time to collision(s)*/
  car_move_side car_shift_info;
} sf_xcb_ldw_output_t;

/************************************************************************/
/* LDW APIs                                                            */
/************************************************************************/

/** Initialize instance of LDW
 *
 *  @param [in] params: Parameters of LDW
 *  @param [in] mapPath: File path of LDW mapping model
 *
 *  @return Pointer to the created LDW module
 */

void *sf_xcb_ldw_init(sf_xcb_ldw_params_t *params, const char *mapPath,const char *mapPath_default,const char *pAuthFilePath);

/** Deallocate LDW and free resources
 *
 *  @param [in] ppLDW: Address of pointer of LDW module
 *
 *  @return Status. None zero if error
 */
int sf_xcb_ldw_release(void **ppLDW);

/** Process new video frame and get LDW result
 *
 *  @param [in] pLDW: Pointer to the created LDW module
 *  @param [in] pImg_back: frame of back-side camera(YUV422_NV61)
 *  @param [in] pImg_left: frame of left-side camera(YUV422_NV61)
 *  @param [in] pImg_front: frame of front-side camera(YUV422_NV61)
 *  @param [in] pImg_right: frame of right-side camera(YUV422_NV61)
 *  @param [in] fSpeed: Speed of car
 *  @param [in] timeStampMs: Timestamp of frame in milliseconds
 *  @param [out] output: Returned pointer to LDW output. Must not be freed!
 *
 *  @return void
 */
void sf_xcb_ldw_process(void *pLDW_, unsigned char *pImg_back,
	unsigned char *pImg_left, unsigned char *pImg_front,
	unsigned char *pImg_right, float fSpeed,
	long long timeStampMs,
	sf_xcb_ldw_output_t **output, unsigned char *stichImg);

void sf_xcb_ldw_process(void *pLDW, unsigned char *pImg_back,
                        unsigned char *pImg_left, unsigned char *pImg_front,
                        unsigned char *pImg_right, float fSpeed,
                        long long timeStampMs,sf_xcb_ldw_output_t **output);
#endif // __SF_XCB_LDW_H__
