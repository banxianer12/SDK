include Rules.mk

ANDROID_UTILS_DEF = -DHAVE_PTHREADS -DHAVE_SYS_UIO_H -DANDROID_SMP=1 -D__ARM_HAVE_DMB \
					-D__ARM_HAVE_LDREX_STREX -DHAVE_POSIX_CLOCKS -DHAVE_PRCTL
SDKLIB_DEF = -DWATERMARK -DCDX_V27 -DSUPPORT_NEW_DRIVER
CEDAR_MACORS = -D_GNU_SOURCE -DCONFIG_CHIP=7 -DCONFIG_TARGET_PRODUCT= -DCONFIG_PRODUCT=2 \
			   -DCONFIG_VE_IPC=1 -DROTATE_PIC_HW=0 -DVE_PHY_OFFSET=0x40000000 -DCONFIG_LOG_LEVEL=0
DEFINES += $(ANDROID_UTILS_DEF) $(SDKLIB_DEF)

INCLUDES = -I./ -I$(SDK_PATH)/cedarx/include/libcedarc/include -I$(SDK_PATH)/include/disp2/ \
        -I$(SDK_PATH)/include/ -I$(SDK_PATH)  -I$(SDK_PATH)/sdk_memory

CFLAGS  += -g -Wall -O3 $(DEFINES) $(INCLUDES) `pkg-config gstreamer-1.0 --cflags` `pkg-config glib-2.0 --cflags`

LIBS    += `pkg-config gstreamer-1.0 --libs` `pkg-config gstreamer-app-1.0 --libs` \
		   -lMemAdapter -lvdecoder -lVE -lcdc_base -lvideoengine  -lcdx_common -lcdx_base \
		   -lsdk_memory -lsdk_disp -lsdk_log -lcdx_ion -lasound -lsdk_decoder \
		   -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lpthread

LDFLAGS += -L$(SDK_PATH) -L$(SDK_PATH)/lib64 -L$(SDK_PATH)/cedarx/lib \
		   -L$(SDK_PATH)/sdk_memory -L$(SDK_PATH)/sdk_decoder -Wl,-rpath-link=$(SDK_PATH)/cedarx/lib

VPATH    = .
OBJS     = parameter_parser.o rtsp_camera.o disp_dev.o display.o sobel.o

TARGETS  = gst_rtsp_dec_cv_edge

# Target rules
all : $(TARGETS)

gst_rtsp_dec_cv_edge:main.o $(OBJS)
	$(CPP) -o $@ $^ $(LDFLAGS) $(LIBS)

%.o:%.c
	$(CC) -c $< -o $@ $(INCLUDES) $(CFLAGS)

%.d:%.c
	@set -e; $(CC) -MM $< $(INCLUDES) > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.o:%.cpp
	$(CPP) -c $< -o $@ $(INCLUDES) $(CFLAGS)

%.d:%.cpp
	@set -e; $(CPP) -MM $< $(INCLUDES) > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

sinclude $(OBJS:.o=.d)

.PHONY:
clean:
	rm -f $(TARGETS) *.o *.d *.d.*
