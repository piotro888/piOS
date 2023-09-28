CC := ~/llvm-project/rbuild/bin/clang
LD := ~/llvm-project/rbuild/bin/ld.lld
AS := ~/llvm-project/rbuild/bin/llvm-mc
OBJCOPY := ~/llvm-project/rbuild/bin/llvm-objcopy
UPLOAD := python3 ../tools/send.py

CFLAGS := -target pcpu-unknown-none -integrated-as -I. -O2 -fno-signed-char  -ffreestanding -nostdlib -nostdinc -fno-zero-initialized-in-bss -fno-delete-null-pointer-checks
CFLAGS_W := -Wall -Wextra -pedantic -Wno-strict-prototypes -Wno-language-extension-token -Wno-gnu-binary-literal -Wno-language-extension-token -Wno-gnu-zero-variadic-macro-arguments
SFLAGS := --arch pcpu --filetype=obj
LFLAGS := -T link.ld


SOURCES=\
	boot.S \
	kstart.c \
	panic.c \
	driver/keyboard.c \
	driver/sd.c \
 	driver/tty.c \
	driver/serial.c \
	driver/spi.c \
	driver/vga.c \
	fs/dirtree.c \
	fs/kbd.c \
	fs/rootfs.c \
	fs/sio.c \
	fs/tar.c \
	fs/tmp.c \
	fs/vfs.c \
	fs/vfs_async.c \
	irq/interrupt.c \
	irq/irq.S \
	irq/timer.c \
	libk/assert.c \
	libk/atomic.c \
	libk/kmalloc.c \
	libk/kprintf.c \
	libk/list.c \
	libk/log.c \
	libk/string.c \
	libk/ringbuff.c \
	libk/con/blockq.c \
	libk/con/semaphore.c \
	libk/con/spinlock.c \
	proc/cswitch.S \
	proc/elf.c \
	proc/sched.c \
	proc/signal.c \
	proc/virtual.c\
	sys/sysd.c

SRC_LIBS=../lib/libc/build/libc_k.a
INC_LIBS=-I../lib/libc/include


OBJS=$(patsubst %, build/%.o, $(SOURCES))

.PHONY: all clean bin upload

all: bin

bin: build/kernel_text.bin build/kernel_data.bin

build/kernel_text.bin: build/kernel.elf
	$(OBJCOPY) --dump-section .text=$@ $< 

build/kernel_data.bin: build/kernel.elf
	$(OBJCOPY) --dump-section .data=$@ $<

build/kernel.elf: $(OBJS) $(SRC_LIBS)
	$(LD) $^ -o $@ $(LFLAGS)

build/%.c.o : %.c
	@mkdir -p $(@D)
	$(CC) $< -o $@ -c $(CFLAGS) $(CFLAGS_WNO) $(INC_LIBS)

build/%.s.o: %.s
	@mkdir -p $(@D)
	$(AS) $< -o $@ $(SFLAGS)

build/%.S.o: build/%.S
	@mkdir -p $(@D)
	$(AS) $< -o $@ $(SFLAGS)

build/%.S: %.S
	@mkdir -p $(@D)
	$(CC) -E $< > $@ -target pcpu

clean:
	rm -rf build/

upload: build/kernel_text.bin build/kernel_data.bin
	$(UPLOAD) $^
