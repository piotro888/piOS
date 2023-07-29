; prologues for compiler generated crtbegin(constructor and desctructors, C code)

.section .init
.global _init
_init:
	sto rc, sp, 0x0
	sto fp, sp, -0x2
	mov fp, sp
	adi sp, sp, -0x4
   /* crtbegin.o .init section here */

.section .fini
.global _fini
_fini:
	sto rc, sp, 0x0
	sto fp, sp, -0x2
	mov fp, sp
	adi sp, sp, -0x4
   /* crtbegin.o .fini section here */
