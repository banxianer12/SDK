################################################################################
#
# toolchain-external-linaro-arm
#
################################################################################

TOOLCHAIN_EXTERNAL_LINARO_ARMSF_VERSION = 2018.05
TOOLCHAIN_EXTERNAL_LINARO_ARMSF_SITE = https://releases.linaro.org/components/toolchain/binaries/7.3-$(TOOLCHAIN_EXTERNAL_LINARO_ARMSF_VERSION)/arm-linux-gnueabi

ifeq ($(HOSTARCH),x86)
TOOLCHAIN_EXTERNAL_LINARO_ARMSF_SOURCE = gcc-linaro-7.3.1-$(TOOLCHAIN_EXTERNAL_LINARO_ARMSF_VERSION)-i686_arm-linux-gnueabi.tar.xz
else
TOOLCHAIN_EXTERNAL_LINARO_ARMSF_SOURCE = gcc-linaro-7.3.1-$(TOOLCHAIN_EXTERNAL_LINARO_ARMSF_VERSION)-x86_64_arm-linux-gnueabi.tar.xz
endif

$(eval $(toolchain-external-package))
