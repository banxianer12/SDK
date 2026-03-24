
!!!使用本库前应仔细研读以下说明及注意事项


1.renderengine库作用（目的）：
进行常用的2D渲染操作，如旋转，平移，缩放, 裁剪等

!!!特别注意，由于egl/opengles是单线程的，因此以下接口必须在同一个线程中运行。

2.头文件说明：
本设计同时兼容C和C++语言编程

如果用户使用的是c++编程，则建议包含renderengine.h和renderengine_uapi.h,
然后使用里面定义的RenderEngine类接口；

如果用户使用的是c编程，则建议包含renderengine_uapi.h，使用里面的接口即可

用户必须包含drm_fourcc.h文件，以便设置像素格式

4.接口调用流程说明
1）在屏渲染
renderEngineCreate() //创建render engine
--->renderEngineSetSurface()--->...--->renderEngineSetSurface() //设置图层，可以设置多个，最后合成一个图层
--->renderEngineDrawOnScreen() //绘制

2)离屏渲染
renderEngineCreate() //创建render engine
---> renderEngineSetOffScreenTarget()---> //设置渲染的target buffer
renderEngineSetSurface()--->...--->renderEngineSetSurface() //设置图层，可以设置多个,最后合成一个图层
--->renderEngineDrawOnScreen() //开始绘制
--->renderEngineWaitSync() //阻塞等待绘制完毕


5.接口详细说明
1)RenderEngine_t renderEngineCreate(bool onScreen, unsigned char debug,
			unsigned int srcFormat, unsigned int dstFormat)
作用：创建并初始化一个render engine
参数：
@onScreen：是否采用在屏方式渲染
@debug: 是否打开库的打印
@srcFormat：输入源图片（纹理）的像素格式，注意必须使用drm 格式
@dstFormat：输出源图片（纹理）的像素格式，注意必须使用drm 格式

2)RenderEngine_t renderEngineDestroy(bool onScreen)
作用：销毁一个render engine

void renderEngineSetSurface(RenderEngine_t RE,
			struct RESurface *surface);
作用：设置一个要进行渲染的图层
参数说明：
struct RESurface {
	int srcFd; //输入图片的内存fd
	unsigned int srcDataOffset; //输入图片的数据在内存中的初始偏移

	unsigned int srcWidth, srcHeight //输入图片的长宽;

	struct RE_rect srcCrop; //对输入图片进行裁剪

	union {
		struct RE_rect dstWinRect; //设置输出图片应该渲染在屏幕的哪一块区域
		struct RE_rect reserved0; //used by renderEngineDrawLayerOnScreenAtCentral()
	};

	float rotateAngle; //旋转角度，逆时针为正
};


3)int renderEngineDrawOnScreen(RenderEngine_t RE)
作用：将renderEngineSetSurface()设置的所有图层渲染到fbdev中


4)int renderEngineDrawOnScreenAtCentral(RenderEngine_t RE,
	 unsigned char rotateType);

作用：此接口是为投屏项目设计的专用接口，只能渲染一个图层，这个图层总位于屏幕正中心位置，
可以旋转任意角度，可以设置旋转过程中自动缩放的效果。

@rotateType:旋转类型，共4种：不旋转、旋转时自动适应屏幕（以屏幕长宽比例进行缩放）、90/270度时铺满屏幕、
旋转时自动适应屏幕（以图层长宽比例进行缩放）。！！！推荐使用最后一种旋转方式

注意:
(1)用户如果使用在屏渲染，则不必考虑渲染同步、显示同步问题
(2)用户如果使用在屏渲染，不必关心送显问题，此接口自动送显
(3)此接口内部有一个3-buffer缓冲队列，假如用户调用此接口时，3-buffer缓冲队列没有满，则
此接口会直接返回，不会阻塞；假如用户调用此接口时，3-buffer缓冲队列满了，此接口会阻塞
到缓冲队列中有空位为止


5)int renderEngineDrawOffScreen(RenderEngine_t RE, void **sync);
作用：以离屏方式渲染多个图像
参数说明：
@sync:渲染同步符号，此接口是非阻塞的,因此需要一个同步信号.
注意！！！由于在linux 上没有采用fence，因此此同步信号不能跨进程传递

6)int renderEngineDrawOffScreenAtCentral(
		      RenderEngine_t RE,
		      unsigned char rotateType,
		      void **sync);
作用：此接口是为投屏项目设计的专用接口，只能渲染一个图层，这个图层总位于屏幕正中心位置，
可以旋转任意角度，可以设置旋转过程中自动缩放的效果。

@rotateType:旋转类型，共4种：不旋转、旋转时自动适应屏幕（以屏幕长宽比例进行缩放）、90/270度时铺满屏幕、
旋转时自动适应屏幕（以图层长宽比例进行缩放）。！！！推荐使用最后一种旋转方式

@sync:渲染同步符号，此接口是非阻塞的,因此需要一个同步信号.
注意！！！由于在linux 上没有采用fence，因此此同步信号不能跨进程传递

7)int renderEngineWaitSync(RenderEngine_t RE, void *sync);
作用：阻塞等待渲染完成


3.其它注意事项
1）如果采用在屏渲染，那么gpu会自动绑定fbdev，渲染结果会直接输出到fbdev中，因此
其它图层在送显时切不可占用fbdev的通道

2)功能依赖于平台和对应sdk，特定平台不一定全部都支持。具体请以实测为准。

3)代码运行前提：
首先，要保证gpu内核驱动已经加载（如mali.ko）;
其次，要保证/usr/lib或/usr/local/lib等库目录下有，EGL/GLES库

4）如果运行demo过程出现 dma import等错误打印，可能是Mali.so库没有支持dma纹理导入功能，
需要找gpu的同事提供具有完整功能的Mali.so库

3)测试demo使用:
首先要保证gpu的驱动已经加载；
其次libEGL.so libopenGLESv2.so等库均已被推送；
再者，需要修改render_demo.c的相关运行参数（在代码里修改）；
再者，根据系统的实际情况修改render_demo.c中ion的堆的申请类型；
再者，需要在demo运行目录下，创建一个pic_bin目录，然后把测试文件1920_1080.nv12导入这个目录，然后运行demo即可

4.性能描述
1)格式转换支持
输入支持：rgba8888/nv21/nv12/yv12/nv61/nv16/yv16 客户可以根据自己需求自己往代码里面添加，添加方式详见第5章
输出支持：rgba8888/nv21/nv12/yv12/nv61/nv16/yv16 客户可以根据自己需求自己往代码里面添加，添加方式详见第5章

5.如何让库支持新的像素格式
(1)在全局数组变量DrmFormat[]中添加新的像素格式，方便调试
(2)在creatEglImage()函数中添加新的像素格式的像素排布特点，用于创建eglImage，以创建相应格式的纹理与fbo
(3)在is_yuv_format()函数中添加新的像素格式，用于选择shader

注意！！！如果出现以下log，说明gpu的ddk不支持新添加的像素格式
[RE ERROR]EGL error state: EGL_BAD_MATCH (0x3009)
[RE ERROR]creatEglImage failed
[RE ERROR]createFBO failed
