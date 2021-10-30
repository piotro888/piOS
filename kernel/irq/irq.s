.romd

; interrupt vector (0x01 @ boot.s) jumps here
interrupt_handler:
    ; user space not yet supported - interrupts from kernel space use r7 as kernel sp
    ; start from -3 to not break prologue/epilogue

    ; save registers
    sto r0, r7, -3
    sto r1, r7, -4
    sto r2, r7, -5
    sto r3, r7, -6
    sto r4, r7, -7
    sto r5, r7, -8
    sto r6, r7, -9

    ; save interrupted program counter
    srl r0, 3
    sto r0, r7, -10
    ; save arithmetic flags (before any arith operation!)
    srl r0, 4
    sto r0, r7, -11
    ; adjust sp
    adi r7, r7, -12
    ; state saved

    jal r6, interrupt ; call kernel interrupt handler

    ; return from interrupt
    
    adi r7, r7, 12
    ldo r0, r7, -11
    srs r0, 4 ; arithmetic flags
    ldo r0, r7, -10
    srs r0, 3 ; return program counter
    ; restore registers
    ldo r0, r7, 0
    ldo r1, r7, -3
    ldo r2, r7, -4
    ldo r3, r7, -5
    ldo r4, r7, -6
    ldo r5, r7, -7
    ldo r6, r7, -8
    ; return from interrupt
    irt
