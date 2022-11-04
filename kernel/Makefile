CC := ~/opt/gcc-pcpu/bin/pcpu-unknown-elf-gcc 
ASM := python3 ~/opt/pcpu-tools/pas.py
ASMCONV := python3 ~/opt/pcpu-tools/asmconv.py
SEND := python3 ../tools/send.py
PEMU := ~/pcpu/toolchain/pcpuemu
PCE := ~/opt/pcpu-tools/pce

CFLAGS := -S -Wall -Wextra -pedantic -I.

AOBJS=\
	boot.ao\
	kstart.ao\
	panic.ao\
	driver/keyboard.ao\
	driver/sd.ao\
	driver/serial.ao\
	driver/spi.ao\
	driver/tty.ao\
	driver/vga.ao\
	fs/dirtree.ao\
	fs/kbd.ao\
	fs/tar.ao\
	fs/vfs.ao\
	irq/interrupt.ao\
	irq/irq.ao\
	irq/timer.ao\
	libk/assert.ao\
	libk/atomic.ao\
	libk/kmalloc.ao\
	libk/kprintf.ao\
	libk/list.ao\
	libk/log.ao\
	libk/ringbuff.ao\
	libk/string.ao\
	libk/con/blockq.ao\
	libk/con/semaphore.ao\
	libk/con/spinlock.ao\
	proc/cswitch.ao\
	proc/elf.ao\
	proc/sched.ao\
	proc/virtual.ao\
	sys/sysd.ao\
	sys/sysres.ao

.PHONY: all clean

all: kernel.hex

upload: kernel.pbl
	$(SEND) build/kernel.pbl

kernel.pbl: $(AOBJS)
	cd build && cat $(AOBJS) > kernel.ao 
	$(ASM) -b build/kernel.ao -o build/kernel.pbl -m

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

emu: kernel.hex kernel.pbl
	$(PEMU) build/kernel.hex build/kernel.pbl

pce: kernel.pbl
	$(PCE) build/kernel.pbl build/kernel.pbl.mm ../test.tar --run
