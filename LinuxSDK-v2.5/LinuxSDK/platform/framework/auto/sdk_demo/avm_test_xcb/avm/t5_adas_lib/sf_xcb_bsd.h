#ifndef __SF_XCB_BSD_H__
#define __SF_XCB_BSD_H__

/************************************************************************/
/* BSD declaration                                                     */
/************************************************************************/
/** The type of BSD warning event */
typedef enum sf_xcb_bsd_event_s
{
  SF_XCB_BSD_No_Warning = 0,        //No obj find on both side (*车左右无目标*)
  SF_XCB_BSD_Left_Warning = 1,      //find obj  in car left  side (*车左边发现目标*)
  SF_XCB_BSD_Right_Warning = 2,     //find obj  in car Right  side (*车右边发现目标*)
  SF_XCB_BSD_Left_Right_Warning = 3 //find obj  on both side (*车左右边发现目标*)
} sf_xcb_bsd_event_t;

/** The type of BSD warning output */
typedef struct sf_xcb_bsd_output_s
{
  void *positions;
  int nRect;
} sf_xcb_bsd_output_t;
/************************************************************************/
/* BSD APIs                                                            */
/************************************************************************/

/** Initialize instance of BSD
 *
 *  @param [in] fActSpeed: Activated speed for BSD
 *  @param [in] dist_horizontal: Detection distance in horizontal direction
 *  @param [in] dist_vertical: Detection distance in vertical direction
 *	@param [in] width: Width of input gray image
 *  @param [in] height: Height of input gray image
 *
 *  @return Pointer to the created BSD module
 */
void *sf_xcb_bsd_init(float fActSpeed, int dist_horizontal,
                      int dist_vertical, int width, int height, const char *pCalibOutPath, const char *pCalibOutPath_Default, const char *pAuthFilePath);

/** Deallocate BSD and free resources
 *
 *  @param [in] ppBSD: Address of pointer of BSD module
 *
 *  @return Status. None zero if error
 */
int sf_xcb_bsd_release(void **ppBSD);

/** Process new video frame and get BSD result
 *
 *  @param [in] pBSD: Pointer to the created BSD module
 *  @param [in] gray_back: gray frame of back-side camera
 *  @param [in] gray_left: gray frame of left-side camera
 *  @param [in] gray_front: gray frame of front-side camera
 *  @param [in] gray_right: gray frame of right-side camera
 *  @param [in] fSpeed: Speed of car
 *  @param [out] event: Returned pointer to BSD event. Must not be freed!
 *
 *  @return void
 */
void sf_xcb_bsd_process(void *pBSD, unsigned char *gray_back,
                        float fSpeed, sf_xcb_bsd_event_t **event);

//use for debug
void sf_xcb_bsd_get_objects(void *pBSD, sf_xcb_bsd_output_t **obj);

void sf_xcb_bsd_get_detect_objects(void *pBSD,
                                   sf_xcb_bsd_output_t **obj);
#endif /*__FORYOU_XCB_BSD_H__*/
