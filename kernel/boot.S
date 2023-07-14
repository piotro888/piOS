#ifdef PAS_DIRS
.romd
#else
.text
#endif

;.rod magicnumber, 0xCAFE

; begin of piOS ^^

#ifndef PAS_DIRS
.global _start
.section .init
_start:
#endif
zero:

jmp _startpoint ; reset vector

jmp interrupt_handler ; interrupt vector (0x1) -> irq/irq.s
jmp core1_interrupt_handler

_startpoint:
	; set up initial stack
	ldi r7, 0xFFFE
	ldi r5, 0xFFFE
	
	; mark r0
	ldi r0, 0x314

	; set cpu flags: privileged mode
	ldi r0, 0x1
	srs r0, 1

	srl r0, 0b1000 ; core number
	cmi r0, 0b1
	jeq c1start
	
	; call _start in kstart.c
	jal r6, _kstart	

	c1start:
	ldi r7, 0xEFFE
	ldi r5, 0xEFFE
	jmp c1start

	; kernel should not return
	; error code
	ldi r0, 0xAF
	ASMerr_loop:
		jmp ASMerr_loop

#ifdef PAS_DIRS
.ramd
.org 0x5000
.romd
#endif
