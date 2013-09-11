BASE := $(shell readlink -f ../..)

OPENCL ?= OpenCL

CC      := gcc
CFLAGS  := -g -std=gnu99 -Wall -I../../include
LDFLAGS := -g -L$(BASE)/lib
LDLIBS  := -l$(OPENCL) -lpancake

BINF=mmul
OBJS=$(shell ls *.c | perl -pe 's/\.c\b/.o/')

all:
	./generate_perror.pl
	make $(BINF)

$(BINF): $(OBJS)

$(OBJS): %.o: %.c *.h pclu_perror.h

pclu_perror.c pclu_perror.h: generate_perror.pl
	./generate_perror.pl

clean:
	rm -f *.o *~ $(BINF) pclu_perror.c pclu_perror.h

.PHONY: clean
