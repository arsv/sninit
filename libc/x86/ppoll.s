.equ NR_ppoll, 309
.equ sizeof_sigset_t, 8

.text
.global ppoll

ppoll:
	movl	$sizeof_sigset_t, 5*4(%esp)
	mov	$NR_ppoll, %ax
	jmp	unisysx

.type ppoll,@function
.size ppoll,.-ppoll
