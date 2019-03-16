.PHONY: all clean test

CC         := gcc -Wall
RM         := rm
LDFLAGS    := -lmagic -lavformat -lswscale $(shell pkg-config --libs GraphicsMagick)
CFLAGS     := -g3 -O0 -fPIC -Iinclude $(shell pkg-config --cflags GraphicsMagick)
SRC_FILES  := $(foreach file,$(notdir $(wildcard src/*.c)),src/$(file))
TEST_FILES := $(foreach file,$(notdir $(wildcard test/*.c)),test/$(file))
SRC_OBJS   := $(SRC_FILES:.c=.o)
TEST_OBJS  := $(TEST_FILES:.c=)
LIB_NAME   := image_proc
LIB_OBJ    := lib$(LIB_NAME).so

all: $(LIB_OBJ)

clean:
	$(RM) -fr $(SRC_OBJS) $(TEST_OBJS) $(LIB_OBJ)

test: $(TEST_OBJS)

test/%: test/%.c $(LIB_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ -Wl,-rpath . -L. -l$(LIB_NAME)

$(LIB_OBJ): $(SRC_OBJS)
	$(CC) $(LDFLAGS) $(SRC_OBJS) -shared -o $(LIB_OBJ)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

src/fingerprint.c: src/fingerprint.gperf
	 gperf -CGD $< -t -K name -N mime_lookup > $@
