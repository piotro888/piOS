CC := ~/opt/gcc-pcpu/bin/pcpu-unknown-elf-gcc 
ASM := python3 ~/opt/pcpu-tools/pas.py
ASMCONV := python3 ~/opt/pcpu-tools/asmconv.py

CFLAGS := -S -Wall -Wextra -pedantic -I.

NAME := example

BIN_TARGET := /bin/example

AOBJS=\
    crt0.ao\
    main.ao

.PHONY: elf clean copy

copy: elf
	mkdir -p ../../base${BIN_TARGET}
	cp build/${NAME} ../../base${BIN_TARGET}

elf: $(AOBJS)
	cd build && cat $(AOBJS) > elfcat.ao 
	$(ASM) build/elfcat.ao -o build/${NAME} --elf-exec
	chmod +x build/${NAME}

%.ao: %.c
	mkdir -p build/$(@D)
	$(CC) $< $(CFLAGS) -o build/$@.tmp
	$(ASMCONV) build/$@.tmp > build/$@

%.ao: %.s
	mkdir -p build/$(@D)
	cp $< build/$@

clean:
	rm -rf build/

