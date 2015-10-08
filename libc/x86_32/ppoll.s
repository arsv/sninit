.equ NR_ppoll, 271
.equ sizeof_sigset_t, 8

.text
.global ppoll

ppoll:
	mov	$sizeof_sigset_t, %r8d
	mov	$NR_ppoll, %ax
	jmp	_syscallx

.type ppoll,@function
.size ppoll,.-ppoll
