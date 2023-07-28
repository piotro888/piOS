CC := ~/llvm-project/rbuild/bin/clang
AR := ~/llvm-project/rbuild/bin/llvm-ar

CFLAGS := -target pcpu -integrated-as -Iinclude/ -fno-signed-char -std=c99 -ffreestanding -Wall -Wextra -pedantic 
CFLAGS_WNO :=  -Wno-strict-prototypes -Wno-language-extension-token -Wno-gnu-binary-literal -Wno-language-extension-token -Wno-gnu-zero-variadic-macro-arguments
LIBC_FLAGS := -D __LIBC__ -D __LIBC_USERSPACE__
LIBC_K_FLAGS := -D __LIBC__ -D __LIBC_KERNEL__


_SOURCES=\
	string.c	


OBJS=$(patsubst %, build/%.o, $(_SOURCES))
OBJS_K=$(patsubst %, build/%_k.o, $(_SOURCES))
SOURCES=$(patsubst %, src/%, $(_SOURCES))


.PHONY: all clean libc libc_k

all: libc libc_k

libc: build/libc.a
libc_k: build/libc_k.a

build/libc.a: $(OBJS)
	ar rcs $@ $^

build/libc_k.a: $(OBJS_K)
	ar rcs $@ $^

build/%.c.o : src/%.c
	@mkdir -p $(@D)
	$(CC) $< -o $@ -c $(CFLAGS) $(CFLAGS_WNO) $(LIBC_FLAGS)

build/%.c_k.o : src/%.c
	@mkdir -p $(@D)
	$(CC) $< -o $@ -c $(CFLAGS) $(CFLAGS_WNO) $(LIBC_K_FLAGS)

clean:
	rm -rf build/
