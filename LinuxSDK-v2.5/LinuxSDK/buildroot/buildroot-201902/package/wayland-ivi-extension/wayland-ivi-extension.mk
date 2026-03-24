################################################################################
#
# wayland-ivi-extension
#
################################################################################

WAYLAND_IVI_EXTENSION_VERSION = 2.1.0
ifeq ($(BR2_AW_WAYLAND_IVI_EXTENSION),y)
WAYLAND_IVI_EXTENSION_SITE_METHOD = local
WAYLAND_IVI_EXTENSION_SITE = $(TOPDIR)/../../platform/framework/wayland-ivi-extension
else
WAYLAND_IVI_EXTENSION_SITE = http://github.com/GENIVI/wayland-ivi-extension/releases
WAYLAND_IVI_EXTENSION_SOURCE = wayland-ivi-extension-$(WAYLAND_IVI_EXTENSION_VERSION).tar.gz
endif
WAYLAND_IVI_EXTENSION_LICENSE = MIT
WAYLAND_IVI_EXTENSION_LICENSE_FILES = COPYING
WAYLAND_IVI_EXTENSION_INSTALL_STAGING = YES
WAYLAND_IVI_EXTENSION_DEPENDENCIES = weston


# wayland-scanner is only needed for building, not on the target
WAYLAND_IVI_EXTENSION_CONF_OPTS = -DBUILD_ILM_API_TESTS=1

$(eval $(cmake-package))
