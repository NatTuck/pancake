
CC=g++
CXXFLAGS=-g -fPIC -Wall -I../include 
LDFLAGS=-g -fPIC -shared 
LDLIBS=-lOpenCL `../scripts/llvm-libs.pl`

HDRS=$(ls ../include/*.hh)
OBJS=$(shell ls *.cc | perl -pe 's/\.cc\b/.o/')

libpancake.so: $(OBJS) Makefile
	$(CC) $(LDFLAGS) -o libpancake.so $(OBJS) $(LDLIBS)

$(OBJS): %.o: %.cc $(HDRS)

clean:
	rm -f *.o *.so
