; epilogues for compiler generated crtend (constructor and desctructors, C code)

.section .init
   /* crtend.o .init section here */
   	mov sp, fp
	ldo fp, sp, -0x2
	ldo rc, sp, 0x0
	srs r6, 0x0

.section .fini
   /* crtend.o .fini section here */
   	mov sp, fp
	ldo fp, sp, -0x2
	ldo rc, sp, 0x0
	srs r6, 0x0