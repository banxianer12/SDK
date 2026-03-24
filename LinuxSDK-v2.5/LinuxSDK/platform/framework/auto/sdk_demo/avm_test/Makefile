#CC:=/home/xufang/workspace/T5anQ/v11/out/gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc
#out/t507/demo2.0/longan/buildroot/host/bin/aarch64-linux-gnu-gcc
#ARCH_DIR=/home/linux01/t5longan06/kernel/linux-4.9/arch/arm64/include/
ARCH_DIR=$(LICHEE_KERN_DIR)/arch/arm64/include/
CFLAGS := -I $(ARCH_DIR)
TARGET := avm_demo

include ../makefile_cfg

SDK_PATH =../../sdk_lib

CEDAR_LINK_NEW = -L$(SDK_PATH) -L$(SDK_PATH)/lib64/ -L$(SDK_PATH)/cedarx/lib/ -Wl,--no-undefined -ladecoder -laencoder -lcdx_base -lcdx_common \
	-lcdx_muxer -lcdx_parser -lcdx_playback -lcdx_stream -lMemAdapter -lcdc_base -lsubdecoder -lvdecoder \
	-lvencoder -lVE -lvideoengine -lxmetadataretriever -lxplayer -lvenc_base -lawh264 -lvenc_codec -lcdx_ion	
CEDAR_MACORS = -D_GNU_SOURCE -DCONFIG_CHIP=7 -DCONFIG_TARGET_PRODUCT= -DCONFIG_PRODUCT=2 \
				-DCONFIG_VE_IPC=1 -DROTATE_PIC_HW=0 -DVE_PHY_OFFSET=0x40000000 -DCONFIG_LOG_LEVEL=0 

DEFINES += -DUSE_LOGCAT
ANDROID_UTILS_DEF = -DHAVE_PTHREADS -DHAVE_SYS_UIO_H -DANDROID_SMP=1 -D__ARM_HAVE_DMB -D__ARM_HAVE_LDREX_STREX -DHAVE_POSIX_CLOCKS -DHAVE_PRCTL
SDKLIB_DEF = -DWATERMARK -DCDX_V27 -DSUPPORT_NEW_DRIVER
DEFINES += $(ANDROID_UTILS_DEF) $(SDKLIB_DEF)

INC = -I$(SDK_PATH)/include \
	-I $(SDK_PATH)/include/opengl/include \
	-I ./avm/t5_adas_lib \
	-I ./avm/t5libavm/t5standard \
	-I $(SDK_PATH)/include/opengl/include/khronos/original/ \
	-I$(SDK_PATH)/include/disp2 \
	-I$(SDK_PATH)/include/utils \
	-I$(SDK_PATH)/include/cutils/ \
	-I$(SDK_PATH)/include/media \
	-I$(SDK_PATH)/include/camera \
	-I$(SDK_PATH)/sdk_camera/moudle \
	-I$(SDK_PATH)/sdk_camera \
	-I$(SDK_PATH)/include/storage/ \
	-I$(SDK_PATH)/include/memory/ \
	-I$(SDK_PATH)/sdk_misc/ \
	-I$(SDK_PATH)/include/audioenc/ \
	-I$(SDK_PATH)/cedarx/include/libcedarc/include/ \
	-I$(SDK_PATH)/cedarx/include/libcore/muxer/include/ \
	-I$(SDK_PATH)/cedarx/include/ \
	-I$(SDK_PATH)/cedarx/include/external/include/aencoder/ \
	-I$(SDK_PATH)/cedarx/include/libcore/base/include/ \
	-I$(SDK_PATH)/cedarx/include/libcore/parser/include/ \
	-I$(SDK_PATH)/cedarx/include/libcore/stream/include/ \
	-I$(SDK_PATH)/eve_module/include \
	-I$(SDK_PATH)/eve_module \
	
INC += $(CEDAR_INC)
LIBS = -L$(SDK_LIBS) -lsdk_compose -lsdk_disp -lsdk_g2d -lsdk_dvr -lsdk_player \
		 -lsdk_log -lsdk_camera -lsdk_memory -lsdk_sound -lsdk_storage \
                -lsdk_audenc -lsdk_cfg -lsdk_ctrl -lsdk_egl -lsdk_misc -lasound		
LIBS += -L$(SDK_PATH)/lib64	-lsdk_egl	
LIBS += -L./avm/t5libavm/t5standard -lnavi
LIBS += -L./avm/t5_adas_lib -lbsd -lfcw -lldw -llight_avg -lmod 
LIBS += -L./avm/libcalib_t5 -lcalib
LIBS +=  $(LICHEE_BR_OUT)/host/aarch64-buildroot-linux-gnu/sysroot/usr/lib/libmali.so.0.20.0 
LIBS +=  -lpthread -lstdc++ -lGLESv2 -lEGL -lpng 

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(TARGET).cpp DmaIon.cpp
	$(CPP) -c DmaIon.cpp -o DmaIon.o $(CFLAGS) $(INC) -g $(DEFINES)
	$(CPP) -c $(TARGET).cpp -o $(TARGET).o $(CFLAGS) $(INC) $(CEDAR_MACORS) $(DEFINES) $(INC) -DSDKLIB_TEST -fpermissive  -g
	$(CPP) -o $(TARGET) DmaIon.o $(TARGET).o $(LIBS) $(CEDAR_LINK_NEW) -lrt -lpthread -lz -lcrypto -lssl

clean:
	rm -rf $(TARGET) *.o

install:
	-cp $(Target) $(INSTATLL_PATH) 