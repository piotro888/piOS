.romd

;.rod magicnumber, 0xCAFE

; begin of piOS ^^

zero:

jmp _startpoint ; reset vector

jmp interrupt_handler ; interrupt vector (0x1) -> irq/irq.s

_startpoint:
	; set up initial stack
	ldi r7, 0xFFFF
	ldi r5, 0xFFFF
	
	; mark r0
	ldi r0, 0x314
	
	; call _start in kstart.c
	jal r6, _kstart	

	; kernel should not return
	; error code
	ldi r0, 0xAF
	ASMerr_loop:
		jmp ASMerr_loop

.ramd
.org 0x4c00
.romd
