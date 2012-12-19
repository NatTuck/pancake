
CC=g++

CFLAGS=-g -Wall -I../../include
LDFLAGS=-g -L../../lib -L$(HOME)/Apps/pocl/lib
LDLIBS=-lpocl -lltdl -lpancake `../../scripts/llvm-libs.pl`
#LDLIBS=-lOpenCL -lpancake `../../scripts/llvm-libs.pl` -lpthread

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
