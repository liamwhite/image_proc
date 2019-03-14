.PHONY: all clean

CC         := gcc
RM         := rm
LDFLAGS    := $(shell pkg-config --libs GraphicsMagick)
CFLAGS     := -Os -fPIC -Iinclude $(shell pkg-config --cflags GraphicsMagick)
SRC_FILES  := $(foreach file,$(notdir $(wildcard src/*.c)),src/$(file))
TEST_FILES := $(foreach file,$(notdir $(wildcard test/*.c)),test/$(file))
SRC_OBJS   := $(SRC_FILES:.c=.o)
TEST_OBJS  := $(TEST_FILES:.c=.o)
LIB_OBJ    := libimage_proc.so

all: $(LIB_OBJ)

clean:
	$(RM) -fr $(SRC_OBJS) $(TEST_OBJS) $(LIB_OBJ)

$(LIB_OBJ): $(SRC_OBJS)
	$(CC) $(LDFLAGS) $(SRC_OBJS) -shared -o $(LIB_OBJ)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

src/fingerprint.c: src/fingerprint.gperf
	 gperf -CGD fingerprint.gperf -t -K name -N mime_lookup > fingerprint.c
