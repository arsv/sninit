# This could have been just
#	mov	r3, #0
#	b	wait4
# as it is done in dietlibc, but the thing is, we don't really need wait4.

.equ NR_wait4, 114

.text
.align	4
.global waitpid

waitpid:
	mov	r3, #0
@wait4:
        stmfd	sp!,{r4,r5,r7,lr}
	ldr	r4, [sp,#16]
	ldr	r5, [sp,#20]
        ldr     r7, =NR_wait4
	b	unisys

.type waitpid,function
.size waitpid,.-waitpid
