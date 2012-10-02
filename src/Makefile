
CFLAGS=-g -fPIC -Wall -std=gnu99 -I../include
LDFLAGS=-g -fPIC -shared
LDLIBS=-lOpenCL

HDRS=$(ls ../include/*.h)
OBJS=$(shell ls *.c | perl -pe 's/\.c\b/.o/')

libpancake.so: $(OBJS) Makefile
	$(CC) $(LDFLAGS) -o libpancake.so $(OBJS) $(LDLIBS)

$(OBJS): %.o: %.c $(HDRS)

clean:
	rm -f *.o *.so
