./csitestT5 3 0 1280 720 ./ 4 10000 25
./csitestT5_ptr 3 0 1280 720 ./ 4 10000 25
3   	/dev/video3
0   	sel
w   	1280
h   	720
./  	path_name
4   	mode
10000  	test_cnt
25		fps


csi_test_mplane_usrptr.c   和  csi_test_mplane.c的不同点在于：
csi_test_mplane_usrptr.c 中cam使用的内存是用户空间（用DmaIon.h的接口）申请的，
所以链接的时候也要把DmaIon.o编出来链接进去。


要编译/清空usrptr版本的话使用如下命令：
make -f Makefile_usrptr 
make -f Makefile_usrptr clean


csi_test_mplane_64bit  是编译好的对应64位软件，用的是mmp方式，和csitestT5一样
csi_test_mplane_32bit  编译好的对应32位软件，用的是mmp方式

测试usb摄像头的方法1：
推webcamtestT5到/usr/bin
chmod 755 /usr/bin/webcamtestT5
反复拔插usb摄像头，检查新生成的摄像头节点/dev/videoX
然后执行
webcamtestT5 0 0 1280 720 /tmp 6 10 25
我这里假设摄像头节点是/dev/video0,然后分辨率是1280x720
如果不知道usb摄像头有什么分辨率，可以先执行一下webcamtestT5，会有对应打印可以看到，比如：
# webcamtestT5 0 0 1280 720 /tmp 6 10 25
open /dev/video0 fd = 3
mCameraType = CAMERA_TYPE_UVC
format index = 0, name = YUYV 4:2:2, v4l2 pixel format = 56595559
format index = 0, name = YUYV 4:2:2, v4l2 pixel format = 56595559
format index = 0, name = YUYV 4:2:2, v4l2 pixel format = 56595559
capture format: V4L2_PIX_FMT_YUYV
resolution got from sensor = 1280*720 num_planes = 0
VIDIOC_STREAMON ok
file length = 1843200 
file start = 0x7fa1e27000 
VIDIOC_STREAMOFF ok
mode 6 test done at the 0 time!!
time cost 1.843099(s)
对应会在/tmp生成一些bin文件

编译webcam对应的应用的方法：
cp -f csi_test_mplane_webcam.c csi_test_mplane.c
make 
更新csitestT5
 
PS:要使用usb摄像头，需要先加载ko文件
insmod /lib/modules/4.9.170/videobuf2-vmalloc.ko 
insmod /lib/modules/4.9.170/uvcvideo.ko 
