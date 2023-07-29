/* Default linker script for pcpu-unknown-pios */

/* 
    PCPU is a Harvard architecute, so data and program sections are separated here by large offset,
    which is ignored when loading to OS (and in assembly).
    This way both memories are linked against 0x0000 in data and instruction memory.
*/

SECTIONS {
    .text : {
        *(.text)
    } > r_instr
    
    .data : {
        *(.data)
        *(.rodata)
        *(.rodata.str1.2)
    } > r_data

    .bss : {*(.bss)} > r_data
    
}

MEMORY {
    r_instr (rx!w)  : ORIGIN = 0x00000000, LENGTH = 0x10000
    /* start data from first page to crash on null ponter */
    r_data  (w!rx)  : ORIGIN = 0x80001000, LENGTH = 0x10000-0x1000
}

ENTRY(__start)
