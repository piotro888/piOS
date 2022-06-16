; 00050004
; 0001000E
.romd

; r0-data* r1-page r2-end_addr r3-off
set_program_mem:
    ; save pc on stack
    sto r6, r7, 0
    ; NOTE: We can't store flags on stack, because flags include paging, which we need to access stack to recover
    srl r4, 1
    mov r6, r4 ; save old flags

    srs r1, 0x10 ; set page zero mapping

    mov r4, r2
    ; r3 - loop counter
    cl_set_program_mem_loop:
        ldo r2, r0, 0 ; load pointer - we need to access memory before setting memory override
        ldi r1, 0b01011 ; flags: SUP, IMO - InstructionMemoryOverride no memory access allowed later, and MEMory PAGing
        srs r1, 1
        sto r2, r3, 0 ; store in new block pointer from r0
        ldi r1, 0b10001 ; diable IMO, MEMPG and enable MEMSTD
        srs r1, 1
        adi r0, r0, 2 ; increment pointer (u16*)
        adi r3, r3, 1 ; increment loop counter
        cmp r3, r4 ; end addr
        jle cl_set_program_mem_loop

    srs r6, 1  ; recover old flags

    ldi r1, 0
    srs r1, 0x10 ; reset page zero mapping

    ldo r6, r7, 0 ; recover pc from stack
    srs r6, 0 ; return

; r0-data* r1-page r2-end_addr r3-off
set_ram_mem:
    sto r6, r7, 0 ; save pc on stack
    srl r4, 1
    mov r6, r4 ; save old flags
    ori r4, r4, 0x8 ; same as before but do not set instr override
    srs r4, 1

    srs r1, 0x10 ; set page zero mapping

    mov r1, r2

   ; r3 - loop counter
    cl_set_ram_mem_loop:
        ldo r2, r0, 0 ; this is fine - buffer not in page0
        sto r2, r3, 0 ; store in new block pointer from r0
        adi r0, r0, 2 ; increment pointer (u16*)
        adi r3, r3, 2 ; increment loop counter
        cmp r3, r1 ; end addr
        jle cl_set_ram_mem_loop

    ; load old flags
    srs r6, 1

    ldi r1, 0
    srs r1, 0x10 ; reset page zero mapping

    ldo r6, r7, 0 ; recover pc from stack
    srs r6, 0 ; return

; set virtual page mapping to values in struct
; @param r0 pointer to mem_pages*16 + prog_pages*16
set_mapping_from_struct:
    ldo r1, r0, 0
    srs r1, 0x10
    ldo r1, r0, 2
    srs r1, 0x11
    ldo r1, r0, 4
    srs r1, 0x12
    ldo r1, r0, 6
    srs r1, 0x13
    ldo r1, r0, 8
    srs r1, 0x14
    ldo r1, r0, 10
    srs r1, 0x15
    ldo r1, r0, 12
    srs r1, 0x16
    ldo r1, r0, 14
    srs r1, 0x17
    ldo r1, r0, 16
    srs r1, 0x18
    ldo r1, r0, 18
    srs r1, 0x19
    ldo r1, r0, 20
    srs r1, 0x1A
    ldo r1, r0, 22
    srs r1, 0x1B
    ldo r1, r0, 24
    srs r1, 0x1C
    ldo r1, r0, 26
    srs r1, 0x1D
    ldo r1, r0, 28
    srs r1, 0x1E
    ldo r1, r0, 30
    srs r1, 0x1F
    ldo r1, r0, 32
    srs r1, 0x20
    ldo r1, r0, 34
    srs r1, 0x21
    ldo r1, r0, 36
    srs r1, 0x22
    ldo r1, r0, 38
    srs r1, 0x23
    ldo r1, r0, 40
    srs r1, 0x24
    ldo r1, r0, 42
    srs r1, 0x25
    ldo r1, r0, 44
    srs r1, 0x26
    ldo r1, r0, 46
    srs r1, 0x27
    ldo r1, r0, 48
    srs r1, 0x28
    ldo r1, r0, 50
    srs r1, 0x29
    ldo r1, r0, 52
    srs r1, 0x2A
    ldo r1, r0, 54
    srs r1, 0x2B
    ldo r1, r0, 56
    srs r1, 0x2C
    ldo r1, r0, 58
    srs r1, 0x2D
    ldo r1, r0, 60
    srs r1, 0x2E
    ldo r1, r0, 62
    srs r1, 0x2F
    
    srs r6, 0 ; return 

; @param r0 ptr to registeres
c_switch:
    ; load arithmetic flags
    ldo r1, r0, 18
    srs r1, 4

    ; store pc to iret scratch register
    ldo r1, r0, 16
    srs r1, 3 

    ; save r0 to scratch reg for later recovery (where normal memory cannot be accessed)
    ldo r1, r0, 0
    srs r1, 6

    ; registers must be recovered from memory before mem paging is enabled
    ldo r1, r0, 2
    ldo r2, r0, 4
    ldo r3, r0, 6
    ldo r4, r0, 8
    ldo r5, r0, 10
    ldo r6, r0, 12
    ldo r7, r0, 14

    ldi r0, 0x19 ; enable rom paging, priv and stdmem mode. *remember about arithmetic flags!*; irq is set by iret
    srs r0, 1

    ldi r0, 0b10
    srs r0, 2 ; set jtr rom paging

    srl r0, 6 ; recover r1 saved before paging enable

    irt ; jump to pc stored in sr3

; Thread to schedule when no process is ready. It is interruptable and not allowed to use memory
idle_task:
    ldi r0, 0
    idle_task_l:
        jmp idle_task_l

.ramd
