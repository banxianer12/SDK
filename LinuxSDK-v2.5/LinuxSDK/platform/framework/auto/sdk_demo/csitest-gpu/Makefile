#CC:=/home/xufang/workspace/T5anQ/v11/out/gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc
#out/t507/demo2.0/longan/buildroot/host/bin/aarch64-linux-gnu-gcc
#ARCH_DIR=/home/linux01/t5longan06/kernel/linux-4.9/arch/arm64/include/
ARCH_DIR=$(LICHEE_KERN_DIR)/arch/arm64/include/
CFLAGS := -I $(ARCH_DIR)
TARGET := csitestT5_ptr

include ../makefile_cfg

SDK_PATH =../../sdk_lib

INC = -I$(SDK_PATH)/include \
	-I $(SDK_PATH)/include/opengl/include \
		-I $(SDK_PATH)/include/opengl/include/khronos/original/
		
LIBS += -L$(SDK_PATH)/lib64		
#LIBS += -law_opengles 
LIBS +=  $(LICHEE_BR_OUT)/host/aarch64-buildroot-linux-gnu/sysroot/usr/lib/libmali.so.0.20.0 
LIBS +=  -lsdk_egl

.PHONY: all clean

all: $(TARGET)

$(TARGET): csi_test_mplane_usrptr.cpp DmaIon.cpp
	$(CPP) -c DmaIon.cpp -o DmaIon.o $(CFLAGS) $(INC) -g
	$(CPP) -c csi_test_mplane_usrptr.cpp -o csi_test_mplane_usrptr.o $(CFLAGS) $(INC) -fpermissive  -g
	$(CC) -o $(TARGET) DmaIon.o csi_test_mplane_usrptr.o $(LIBS)

clean:
	rm -rf $(TARGET) csitestT5 *.o

install:
	-cp $(Target) $(INSTATLL_PATH) 