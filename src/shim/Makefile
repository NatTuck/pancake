
CC=g++
CXXFLAGS=-g -fPIC -Wall -I../../include `llvm-config --cxxflags`
LDFLAGS=-g -fPIC -shared 
LDLIBS=`../../scripts/llvm-libs.pl`

HDRS=$(ls ../../include/pancake/*.hh)
OBJS=$(shell ls *.cc | perl -pe 's/\.cc\b/.o/')

libpancake.so: $(OBJS) Makefile
	$(CC) $(LDFLAGS) -o libpancake.so $(OBJS) $(LDLIBS)

$(OBJS): %.o: %.cc $(HDRS)

clean:
	rm -f *.o *.so
