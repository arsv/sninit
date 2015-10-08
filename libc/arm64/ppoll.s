.equ NR_ppoll, 73
.equ sizeof_sigset_t, 8

.text
.global ppoll

ppoll:
	mov	x4, sizeof_sigset_t
	mov	x8, NR_ppoll
	b	_syscall

.type ppoll,function
.size ppoll,.-ppoll
