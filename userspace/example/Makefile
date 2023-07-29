CC := ~/llvm-project/rbuild/bin/clang
LD := ~/llvm-project/rbuild/bin/ld.lld
AS := ~/llvm-project/rbuild/bin/llvm-mc

CFLAGS := -target pcpu -integrated-as -I. -fno-signed-char -Wall -Wextra -pedantic -I../../lib/libpios
SFLAGS := --arch pcpu --filetype=obj
LFLAGS := -T link.ld

NAME := example

BIN_TARGET := /bin/example

SOURCES=\
    crt0.s\
    main.c\
	crtn.s

OBJS=$(patsubst %, build/%.o, $(SOURCES))

.PHONY: elf clean copy

copy: build/$(NAME)
	cp build/${NAME} ../../base${BIN_TARGET}

build/$(NAME): $(OBJS)
	$(LD) $^ -o $@ $(LFLAGS)

build/%.c.o : %.c
	@mkdir -p $(@D)
	$(CC) $< -o $@ -c $(CFLAGS)

build/%.s.o: %.s
	@mkdir -p $(@D)
	$(AS) $< -o $@ $(SFLAGS)

clean:
	rm -rf build/

