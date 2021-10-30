CC := ~/opt/gcc-pcpu/bin/pcpu-unknown-elf-gcc 
ASM := python3 ~/pcpu/toolchain/pas.py
ASMCONV := python3 ~/pcpu/toolchain/asmconv.py
SEND := python3 ../tools/send.py

CFLAGS := -S -Wall -Wextra -pedantic -I.

AOBJS=\
	boot.ao\
	kstart.ao\
	irq/interrupt.ao\
	irq/irq.ao\
	libk/kprintf.ao\
	driver/keyboard.ao\
	driver/tty.ao

.PHONY: all clean

all: kernel.hex

upload: kernel.pbl
	$(SEND) build/kernel.pbl

kernel.pbl: $(AOBJS)
	cd build && cat $(AOBJS) > kernel.ao 
	$(ASM) -b build/kernel.ao -o build/kernel.pbl

kernel.hex: $(AOBJS)
	cd build && cat $(AOBJS) > kernel.ao 
	$(ASM) build/kernel.ao -o build/kernel.hex

%.ao: %.c
	mkdir -p build/$(@D)
	$(CC) $< $(CFLAGS) -o build/$@.tmp
	$(ASMCONV) build/$@.tmp > build/$@

%.ao: %.s
	mkdir -p build/$(@D)
	cp $< build/$@

clean:
	rm -rf build/
