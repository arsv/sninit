.data
.global errno

errno:	.word 0

.type errno,object
.size errno,.-errno


.text
.align 4

.global unisys
.global errno

unisys:
        cmn     r0, #4096
        rsbcs   r2, r0, #0
        ldrcs   r3, =errno
        mvncs   r0, #0
        strcs   r2, [r3]
        ldmfd   sp!,{r4,r5,r7,pc}
	mov	pc, lr			@ return

.size unisys,.-unisys
.type unisys,function
