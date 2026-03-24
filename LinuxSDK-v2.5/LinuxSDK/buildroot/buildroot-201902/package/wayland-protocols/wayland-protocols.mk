################################################################################
#
# wayland-protocols
#
################################################################################

WAYLAND_PROTOCOLS_VERSION = 1.17

ifeq ($(BR2_AW_WAYLAND_PROTOCOLS),y)
WAYLAND_PROTOCOLS_SITE_METHOD = local
WAYLAND_PROTOCOLS_SITE = $(TOPDIR)/../../platform/framework/wayland-protocols
else
WAYLAND_PROTOCOLS_SITE = http://wayland.freedesktop.org/releases
WAYLAND_PROTOCOLS_SOURCE = wayland-protocols-$(WAYLAND_PROTOCOLS_VERSION).tar.xz
endif

WAYLAND_PROTOCOLS_LICENSE = MIT
WAYLAND_PROTOCOLS_LICENSE_FILES = COPYING
WAYLAND_PROTOCOLS_INSTALL_STAGING = YES
WAYLAND_PROTOCOLS_INSTALL_TARGET = NO

$(eval $(autotools-package))
