test_dir := codec/test
EXTRA_CFLAGS += -I$(src)/$(test_dir)

ifeq (${CONFIG_VIDEO_KERNEL_DEC_SUNXI},y)
test_obj := ${test_dir}/dec_test.o
endif

ifeq (${CONFIG_VIDEO_KERNEL_ENC_SUNXI},y)
test_obj += ${test_dir}/enc_test.o
endif
