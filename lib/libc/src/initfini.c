
extern void __malloc_init();
extern void __stdio_init();

void __libc_init() {
    __malloc_init();
    __stdio_init();
}

extern void __stdio_fini();

void __libc_fini() {
    __stdio_fini();
}

