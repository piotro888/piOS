CC := ~/opt/gcc-pcpu/bin/pcpu-unknown-elf-gcc 
ASM := python3 ~/pcpu/toolchain/pas.py
ASMCONV := python3 ~/pcpu/toolchain/asmconv.py

CFLAGS := -S -Wall -Wextra -pedantic -I.

AOBJS=\
	boot.ao\
	kstart.ao\
	video/tty.ao\

.PHONY: all clean

all: kernel.hex

kernel.hex: $(AOBJS)
	cd build && cat $(AOBJS) > kernel.ao 
	$(ASM) build/kernel.ao -o kernel.hex

%.ao: %.c
	mkdir -p build/$(@D)
	$(CC) $< $(CFLAGS) -o build/$@.tmp
	$(ASMCONV) build/$@.tmp > build/$@

%.ao: %.s
	mkdir -p build/$(@D)
	cp $< build/$@

clean:
	rm -rf build/