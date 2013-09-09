BASE := $(shell readlink -f .)

CC      := gcc
CFLAGS  := -g -fPIC -std=gnu99 -I$(BASE)/include
LDFLAGS := -g -fPIC -shared -L/usr/local/lib
LDLIBS  := -ldrip -lOpenCL

HDRS := $(wildcard include/pancake/*.h)
SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:.c=.o)

LIB := lib/libpancake.so

$(LIB): $(OBJS) Makefile
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(OBJS): $(SRCS) $(HDRS)

clean:
	rm -f $(LIB) $(OBJS)
	find examples -name Makefile -exec bash -c '(cd `dirname {}` && make clean)' \;
