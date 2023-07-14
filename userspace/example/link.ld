SECTIONS {
       .text : {
        *(.init) 
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
    r_data  (w!rx)  : ORIGIN = 0x8000000,  LENGTH = 0x1000000-0x201000
    r_instr (rx!w)  : ORIGIN = 0x0000000, LENGTH = 0x1ffc000-0x1000000
}

ENTRY(_start)
