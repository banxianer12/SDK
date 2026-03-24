TARGET = upgrade
#LIBS = -lm -lz
TOPDIR := $(PWD)/../../../..
TOOLCHAIN := $(TOPDIR)/out/gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu/bin
CC = $(TOOLCHAIN)/aarch64-linux-gnu-gcc
CFLAGS = -g -Wall

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
