.romd

; interrupt vector (0x01 @ boot.s) jumps here
interrupt_handler:
    ; user space not yet supported - interrupts from kernel space use r7 as kernel sp

    ; save registers
    sto r0, r7, 0
    sto r1, r7, -1
    sto r2, r7, -2
    sto r3, r7, -3
    sto r4, r7, -4
    sto r5, r7, -5
    sto r6, r7, -6

    ; save interrupted program counter
    srl r0, 3
    sto r0, r7, -8
    ; save arithmetic flags (before any arith operation!)
    srl r0, 4
    sto r0, r7, -9
    ; adjust sp
    adi r7, r7, -10

    ; state saved

    jal r6, interrupt ; call kernel interrupt handler

    ; return from interrupt
    
    adi r7, r7, 10
    ldo r0, r7, -9
    srs r0, 4 ; arithmetic flags
    ldo r0, r7, -8
    srs r0, 3 ; return program counter
    ; restore registers
    sto r0, r7, 0
    sto r1, r7, -1
    sto r2, r7, -2
    sto r3, r7, -3
    sto r4, r7, -4
    sto r5, r7, -5
    sto r6, r7, -6
    ; return from interrupt
    irt
