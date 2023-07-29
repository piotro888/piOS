CC := ~/llvm-project/rbuild/bin/clang
AS := ~/llvm-project/rbuild/bin/llvm-mc

CFLAGS := -target pcpu-unknown-none -integrated-as -I../libc/include -Wall -Wextra -pedantic -fno-signed-char -ffreestanding -nostdlib -nostdinc
SFLAGS := --arch pcpu --filetype=obj

_TARGETS=\
	crt0.o\
	crti.o\
	crtbegin.o\
	crtend.o\
	crtn.o

TARGETS=$(patsubst %, build/%, $(_TARGETS))

.PHONY: all clean

all: $(TARGETS)

build/%.o : %.c
	@mkdir -p $(@D)
	$(CC) $< -o $@ -c $(CFLAGS) $(INC_LIBS)

build/%.o: %.s
	@mkdir -p $(@D)
	$(AS) $< -o $@ $(SFLAGS)

clean:
	rm -rf build/
