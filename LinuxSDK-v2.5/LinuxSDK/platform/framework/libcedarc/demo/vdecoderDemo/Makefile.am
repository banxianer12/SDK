
include $(top_srcdir)/Makefile.inc

bin_PROGRAMS = vdecoderdemo

vdecoderdemo_SOURCES =  ./vdecoderDemo.c \
            rawStreamParser.c

vdecoderdemo_CFLAGS = $(CFLAGS_CDCG)
#jpegdemo_CPPFLAGS = $(CPPFLAGS_CDXG)
LOCAL_INCLUDE = -I$(top_srcdir) \
                -I$(top_srcdir)/base/include \
                -I$(top_srcdir)/include

vdecoderdemo_CFLAGS += $(LOCAL_INCLUDE)

vdecoderdemo_LDFLAGS = $(LDFLAGS_CDCG)

vdecoderdemo_LDADD = $(top_srcdir)/base/libcdc_base.la \
                            $(top_srcdir)/memory/libMemAdapter.la \
                            $(top_srcdir)/vdecoder/libvdecoder.la

vdecoderdemo_LDFLAGS += $(LOCAL_LIB)

