int main() {
    int x = 0x21;
    __asm__ (
        "mov r0, %0\n"
        "sys\n"
        :: "r"(x) : "r0");

    for(;;);
}
