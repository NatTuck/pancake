BASE := $(shell readlink -f .)

CC      := gcc
CFLAGS  := -g -fPIC -std=gnu99 -Wall -Werror -I/usr/local/include -I$(BASE)/include
LDFLAGS := -g -fPIC -shared -L/usr/lib -L/usr/local/lib
LDLIBS  := -ldrip -lOpenCL -lgc -ljansson

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

prereqs:
	sudo apt-get install libjansson4 libjansson-dev

.PHONY: clean prereqs
