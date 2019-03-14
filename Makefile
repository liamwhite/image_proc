.PHONY: all clean

CC      := gcc
RM      := rm
LDFLAGS := $(shell pkg-config --libs GraphicsMagick)
CFLAGS  := -Os -fPIC $(shell pkg-config --cflags GraphicsMagick)
FILES   := $(notdir $(wildcard *.c))
OBJS    := $(FILES:.c=.o)

all: libimage_proc.so

clean:
	$(RM) -fr $(OBJS)

libimage_proc.so: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -shared -o libimageproc.so

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

fingerprint.c: fingerprint.gperf
	 gperf -CGD fingerprint.gperf -t -K name -N mime_lookup > fingerprint.c
