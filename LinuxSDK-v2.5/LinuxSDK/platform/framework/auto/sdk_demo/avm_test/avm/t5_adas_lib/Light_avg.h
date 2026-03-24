
struct light_init_param
{
  int imgWidth;
  int imgHeight;
  float carLength;      // unit:meter
  float carwidth;       // unit:meter
  float roiw;           // roi 宽
  float roih;           // roi 高
  int adjust_frequency; //>1
};

typedef struct Adjust_Lum_Out
{
  float LineAngle;
  float frontAdjustVal;
  float backAdjustVal;
  float LFAdjustVal;
  float LBAdjustVal;
  float RFAdjustVal;
  float RBAdjustVal;
} adjust_lum_out_t;

/*
 *亮度均衡库 用于调整四个画面显示亮度不一致到一致
 *
 */
/*
  *initlightavg 初始化函数
  *lightavg_param_file 读取亮度均衡求roi区域的标定生成参数模型文件
  *lightavg_defaule_param_file 读取亮度均衡求roi区域的标定前默认参数模型文件
  *in param eight_roi 用于设置roi区域
  *in param row 为图像的高
  *in param cols为图像宽
  *in param adjust_frequency  调整速度帧默认20
  *return (void *) a pointer for lightavg
  */

//为了保证没标定时能使用设置了可传入默认的参数文件；
void *initlightavg(const char *lightavg_param_file, const char *lightavg_defaule_param_file,
                   light_init_param param, const char *auth_file);
/*
 *releaselightavg 资源释放函数
 *in param pplightavg a pointer for lightavg
 *return void
 */
void releaselightavg(void **pplightavg);
/*
 *light_avg_processc 亮度均衡比例计算函数
 *in param pplightavg a pointer for lightavg
 *in param grayB (Y channel of back img)
 *in param grayL (Y channel of left img)
 *in param grayF (Y channel of front img)
 *in param grayR (Y channel of right img)
 *out param float outratio[4]; (B L F R对应的调整比例浮点值)
 *return void
 */
void light_avg_process(void *pLightavg, unsigned char *grayB,
                       unsigned char *grayL, unsigned char *grayF,
                       unsigned char *grayR, adjust_lum_out_t &output,
                       float gmm_lr = 0.1);
