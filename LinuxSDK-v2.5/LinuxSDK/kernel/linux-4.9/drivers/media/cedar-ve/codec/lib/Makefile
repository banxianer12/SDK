lib_dir := codec/lib

EXTRA_CFLAGS += -I$(src)/$(lib_dir)/include

ifeq (${CONFIG_VIDEO_KERNEL_DEC_SUNXI},y)
lib_obj += $(lib_dir)/libvdecoder.a
endif

ifeq (${CONFIG_VIDEO_KERNEL_ENC_SUNXI},y)
lib_obj += $(lib_dir)/libgcc.a
lib_obj += $(lib_dir)/libvencoder.a
endif

lib_obj += $(lib_dir)/libve.a
lib_obj += $(lib_dir)/libmem.a
