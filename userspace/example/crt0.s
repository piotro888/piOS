.global _start
_start:
    ; args and stack is already set by kernel
    jal r6, main

    __end_loop:
        ; TODO: change to exit syscall
        jmp __end_loop
