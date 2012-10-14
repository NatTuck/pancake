
CC=g++

CFLAGS=-g -Wall -I../../include
LDFLAGS=-g -L../../lib
LDLIBS=-lOpenCL -lpancake `../../scripts/llvm-libs.pl`

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
