
ROOT=../..
CXXFLAGS=-g -std=gnu++11 -fPIC -Wall -I$(ROOT)/include `llvm-config --cxxflags`
LDFLAGS=-g -fPIC
LDLIBS=`$(ROOT)/scripts/llvm-libs.pl` -pthread -ldl

BINARY=fry

all: $(BINARY)

$(BINARY): *.cc

clean:
	rm -rf $(BINARY) *.o *~

.PHONY: all clean
