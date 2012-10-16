
all: lib/libpancake.so

lib/libpancake.so: src/libpancake.so
	cp src/libpancake.so lib/libpancake.so

src/libpancake.so:
	(cd src && make)

clean:
	find . -name "*~" -exec rm {} \;
	rm -f lib/libpancake.so
	(cd src && make clean)
	(cd examples/mmul && make clean)

prereqs:
	sudo apt-get install llvm-3.0 llvm-3.0-dev clang opencl-headers

.PHONY: all src/libpancake.so clean prereqs
