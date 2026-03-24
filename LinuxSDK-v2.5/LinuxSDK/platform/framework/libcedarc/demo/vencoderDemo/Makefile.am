
include $(top_srcdir)/Makefile.inc

bin_PROGRAMS = vencoderDemo

vencoderDemo_SOURCES =  \
            EncoderTest.c \
            avtimer.c

vencoderDemo_CFLAGS = $(CFLAGS_CDCG)
LOCAL_INCLUDE = -I$(top_srcdir) \
                -I$(top_srcdir)/base/include \
                -I$(top_srcdir)/include

vencoderDemo_CFLAGS += $(LOCAL_INCLUDE)

vencoderDemo_LDFLAGS = $(LDFLAGS_CDCG)


LOCAL_LIB = -lpthread -lm -lrt -lVE -lvenc_codec

vencoderDemo_LDFLAGS += $(top_srcdir)/base/libcdc_base.la \
                            $(top_srcdir)/memory/libMemAdapter.la \
                            $(top_srcdir)/vencoder/libvencoder.la \
                            $(top_srcdir)/vencoder/base/libvenc_base.la

vencoderDemo_LDFLAGS += $(LOCAL_LIB)
