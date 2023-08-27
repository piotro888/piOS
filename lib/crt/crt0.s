; initialization of program flow and C runtime setup (frame, args, exit and libc init)

.section .text

.global __start
__start:
	; Set up stack frame.
	ldi r5, 0xfffe
    ldi r7, 0xfffe

    ; save argc and argv
    adi r7, r7, -4
    sto r0, r7, 2
    sto r1, r7, 4

    ; interal library init
	jal r6, __libc_init

    ; global constructors
	jal r6, _init

	; restore argc, argv, and full stack
	ldo r0, r7, 2
    ldo r1, r7, 4
    ldi r7, 0xfffe
    
    ; clean regs
    ldi r2, 0
    ldi r3, 0
    ldi r4, 0
    ldi r6, 0

	; and run
	jal r6, main

    jal r6, _fini
    jal r6, __libc_fini

	; TODO: call exit syscall
    exit_loop:
        jmp exit_loop
