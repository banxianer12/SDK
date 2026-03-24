obj-$(CONFIG_VIDEO_SUNXI_CAR_REVERSE) = sunxi_car_reverse.o

ifeq ($(CONFIG_ARCH_SUN50IW9P1),y)
di_dir := drivers/char/sunxi-di/drv_div3x
ccflags-y := -I$(di_dir)

sunxi_car_reverse-y = buffer_pool.o \
	video_source.o \
	preview.o \
	car_reverse.o
else
sunxi_car_reverse-y = buffer_pool.o \
	video_source.o \
	preview.o \
	car_reverse.o
endif


ifeq ($(CONFIG_SUPPORT_AUXILIARY_LINE),y)
sunxi_car_reverse-y += auxiliary_line.o
endif

ifeq ($(CONFIG_SUPPORT_BIRDVIEW),y)
sunxi_car_reverse-y += BVKernal/birdview_algo \
			BVKernal/birdview_api \
			BVKernal/camera \
			BVKernal/fisheye_algo
endif

