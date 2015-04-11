.data
.global errno

errno:	.word 0

.type errno,object
.size errno,.-errno


.text
.align 4

.global unisys
.global unisys5
.global unisys6
.global errno

# Each syscall does
#	stmfd	sp!,{r4,r5,r7,lr}
# before jumping to unisys*, that's 4*4=16 bytes, so the original stack
# content starts at [sp,#16].

unisys6:				/* 6-arg syscall */
	ldr	r5, [sp,#20]
unisys5:				/* 5-arg syscall */
	ldr	r4, [sp,#16]
unisys:					/* 4 or less args */
	swi	0
        cmn     r0, #4096
        rsbcs   r2, r0, #0
        ldrcs   r3, =errno
        mvncs   r0, #0
        strcs   r2, [r3]
        ldmfd   sp!,{r4,r5,r7,pc}
	mov	pc, lr			/* return */

.size unisys,.-unisys
.type unisys,function

.size unisys5,unisys-unisys5
.type unisys5,function

.size unisys6,unisys5-unisys6
.type unisys6,function
