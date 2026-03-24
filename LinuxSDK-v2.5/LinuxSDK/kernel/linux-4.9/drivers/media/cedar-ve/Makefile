EXTRA_CFLAGS += -I$(src)/include

#EXTRA_CFLAGS += -g -O0 -v

obj-$(CONFIG_VIDEO_ENCODER_DECODER_SUNXI) := sunxi-ve.o

sunxi-ve-objs := cedar_ve.o

ifeq (${CONFIG_VIDEO_KERNEL_DEC_SUNXI},y)
obj-$(CONFIG_VIDEO_KERNEL_DEC_ENC_TEST) += dec-test.o
dec-test-objs += test/dec_test.o
include $(src)/codec/Makefile
dec-test-objs += ${codec_obj}
endif

ifeq (${CONFIG_VIDEO_KERNEL_ENC_SUNXI},y)
obj-$(CONFIG_VIDEO_KERNEL_DEC_ENC_TEST) += enc-test.o
enc-test-objs += test/enc_test.o
include $(src)/codec/Makefile
enc-test-objs += ${codec_obj}
endif
