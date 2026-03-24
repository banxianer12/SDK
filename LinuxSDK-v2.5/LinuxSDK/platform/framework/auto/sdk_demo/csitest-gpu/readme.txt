./csitestT5_ptr 3 0 1280 720 ./ 4 10000 25
3   	/dev/video3
0   	sel
w   	1280
h   	720
./  	path_name
4   	mode
10000  	test_cnt
25		fps

gpu测试方法：
1.使用的是video3
2.默认为常规测试，使用csi抓720p数据过来，然后拷贝到gDispTab数组，然后把gDispTab数组通过info_aframe接口送gpu显示


