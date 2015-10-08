.data
.global errno

errno:	.word 0

.type errno,object
.size errno,.-errno


.text
.align 4

.global _syscall
.global _syscall5
.global _syscall6
.global errno

# Each syscall does
#	stmfd	sp!,{r4,r5,r7,lr}
# before jumping to _syscall*, that's 4*4=16 bytes, so the original stack
# content starts at [sp,#16].

_syscall6:				/* 6-arg */
	ldr	r5, [sp,#20]
_syscall5:				/* 5-arg */
	ldr	r4, [sp,#16]
_syscall:				/* 4 or less args */
	swi	0
        cmn     r0, #4096
        rsbcs   r2, r0, #0
        ldrcs   r3, =errno
        mvncs   r0, #0
        strcs   r2, [r3]
        ldmfd   sp!,{r4,r5,r7,pc}
	mov	pc, lr			/* return */

.size _syscall,.-_syscall
.type _syscall,function

.size _syscall5,_syscall-_syscall5
.type _syscall5,function

.size _syscall6,_syscall5-_syscall6
.type _syscall6,function
