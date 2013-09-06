BASE := $(shell readlink -f .)

CC      := gcc
CFLAGS  := -g -fPIC -std=gnu99 -I$(BASE)/include
LDFLAGS := -g -fPIC -shared 
LDLIBS  := -lOpenCL

HDRS := $(wildcard include/pancake/*.h)
SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:.c=.o)

lib/libpancake.so: $(OBJS) Makefile
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(OBJS): $(SRCS) $(HDRS)

clean:
	rm -f $(OBJS) lib/libpancake.so
	(cd examples/mmul && make clean)
