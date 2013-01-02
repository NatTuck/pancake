
DIRS=$(shell find * -maxdepth 0 -type d)

all:
	$(foreach d,$(DIRS),(cd $d && make);)

clean:
	$(foreach d,$(DIRS),(cd $d && make clean);)

.PHONY: all clean
