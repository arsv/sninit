.equ NR_ppoll, 336
.equ sizeof_sigset_t, 8

.text
.align 4
.global ppoll

# ppoll has 5th argument not listed in the prototype sizeof(sigset_t).
# An extra .c file would have solved the problem of course, but why bother?

ppoll:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r4, =sizeof_sigset_t
	ldr	r7, =NR_ppoll
	b	_syscall

.type ppoll,function
.size ppoll,.-ppoll
