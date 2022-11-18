.romd

;.rod magicnumber, 0xCAFE

; begin of piOS ^^

zero:

jmp _startpoint ; reset vector

jmp interrupt_handler ; interrupt vector (0x1) -> irq/irq.s

_startpoint:
	; set up initial stack
	ldi r7, 0xFFFE
	ldi r5, 0xFFFE
	
	; mark r0
	ldi r0, 0x314

	; set cpu flags: privileged mode
	ldi r0, 0x1
	srs r0, 1
	
	; call _start in kstart.c
	jal r6, _kstart	

	; kernel should not return
	; error code
	ldi r0, 0xAF
	ASMerr_loop:
		jmp ASMerr_loop

.ramd
.org 0x5000
.romd
