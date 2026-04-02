LICHEE_DIR = /home/tronlong/T507_SDK/LinuxSDK

include $(LICHEE_DIR)/.buildconfig

LICHEE_OUT_SYS = $(LICHEE_BR_OUT)

SDK_PATH = $(LICHEE_DIR)/platform/framework/auto/sdk_lib

HOST_SYSROOT = $(LICHEE_OUT_SYS)/host/aarch64-buildroot-linux-gnu/sysroot

CC  = $(LICHEE_OUT_SYS)/host/bin/aarch64-linux-gnu-gcc --sysroot=$(HOST_SYSROOT)

CPP = $(LICHEE_OUT_SYS)/host/bin/aarch64-linux-gnu-g++ --sysroot=$(HOST_SYSROOT)

export PKG_CONFIG_SYSROOT_DIR = $(HOST_SYSROOT)
export PKG_CONFIG_PATH = $(HOST_SYSROOT)/usr/lib/pkgconfig
