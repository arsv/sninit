.text
.globl syscall

syscall:
	stmfd	sp!,{r4,r5,r7,lr}
	mov	r7, r0
	mov	r0, r1
	mov	r1, r2
	mov	r2, r3
	ldr	r3, [sp,#16]
	ldr	r4, [sp,#20]
	ldr	r5, [sp,#24]
	b	unisys

.type syscall,function
.size syscall,.-syscall
