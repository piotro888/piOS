#ifdef PAS_DIRS
.romd
#endif

#ifndef PAS_DIRS
.global interrupt_handler
#endif
; interrupt vector (0x01 @ boot.s) jumps here
interrupt_handler:
    ; interrutps from userspace are supported; kernel and userspace interrupts use single kernel stack loaded from memory
    ; start from -3 to not break prologue/epilogue

    ; save registers
    std r7, r7_temp_location ; store r7 to temporary location
    ; load trusted kernel stack pointer (but because of that no nested irqs allowed)
    ldi r7, 0xeff0
    ; save registers to stack
    sto r0, r7, -4
    sto r1, r7, -6
    sto r2, r7, -8
    sto r3, r7, -10
    sto r4, r7, -12
    sto r5, r7, -14
    sto r6, r7, -16
    ; save r7
    ldd r0, r7_temp_location
    sto r0, r7, -18

    ; save interrupted program counter
    srl r0, 3
    sto r0, r7, -20
    ; save arithmetic flags (before any arith operation!)
    srl r0, 4
    sto r0, r7, -22
    ; load special flags useful for kernel
    srl r0, 5
    sto r0, r7, -24
    ; adjust sp (+ space for passing argument!)
    mov r5, r7
    adi r7, r7, -28
    ; state saved

    ; pass pointer to state as argument
    mov r0, r7

    jal r6, interrupt ; call kernel interrupt handler
    
    ; NEVER REACHED. Interrupts should be disabled before sheduler
    ; and we always return to some other thread but not to irq thread (no virtual) when enabled

    ; return from interrupt
    ; but only to kernel (and only main thread, kernel threads also need virutal memory)
    ; returns to userspace via proc/virtual/switch_to_userspace
    ; so this path will be only used in interrupts before scheduler
    ; in that case we could just resume from data stored on stack

    adi r7, r7, 28
    ldo r0, r7, -22
    srs r0, 4 ; arithmetic flags
    ldo r0, r7, -20
    srs r0, 3 ; return program counter
    ; restore registers
    ldo r0, r7, -4
    ldo r1, r7, -6
    ldo r2, r7, -8
    ldo r3, r7, -10
    ldo r4, r7, -12
    ldo r5, r7, -14
    ldo r6, r7, -16
    ldo r7, r7, -18 ; we leave `IRQ stack` now
    ; return from interrupt
    irt

#ifndef PAS_DIRS
.global core1_interrupt_handler
#endif
core1_interrupt_handler:
    jmp core1_interrupt_handler ; TODO

    ; save registers
    std r7, c1_temp_location ; store r7 to temporary location
    ; load trusted kernel stack pointer (but because of that no nested irqs allowed)
    ldi r7, 0xeef0
    ; save registers to stack
    sto r0, r7, -4
    sto r1, r7, -6
    sto r2, r7, -8
    sto r3, r7, -10
    sto r4, r7, -12
    sto r5, r7, -14
    sto r6, r7, -16
    ; save r7
    ldd r0, c1_temp_location
    sto r0, r7, -18

    ; save interrupted program counter
    srl r0, 3
    sto r0, r7, -20
    ; save arithmetic flags (before any arith operation!)
    srl r0, 4
    sto r0, r7, -22
    ; load special flags useful for kernel
    srl r0, 5
    sto r0, r7, -24
    ; adjust sp (+ space for passing argument!)
    mov r5, r7
    adi r7, r7, -28
    ; state saved

    ; pass pointer to state as argument
    mov r0, r7

    ;jal r6, interrupt_c1 ; call kernel interrupt handler

#ifdef PAS_DIRS
.ramd
; stack pointer value loaded after interrupt in kernel
.init interrupt_sp, 0xf0, 0xef ;le ;FIXME: just use kernel sp?
.global c1_temp_location, 2 ; asm bug - when placed one line below
.global r7_temp_location, 2
#else
.data
interrupt_sp:
.byte 0xf0, 0xef
.bss
.lcomm r7_temp_location, 2
.lcomm c1_temp_location, 2
#endif
