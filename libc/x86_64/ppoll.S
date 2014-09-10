#include <bits/syscall.h>

.equ sizeof_sigset_t, 8		// _NSIG/8

.text
.global ppoll

ppoll:
	mov	$__NR_ppoll, %ax
	mov	$sizeof_sigset_t, %r8d
	jmp	unisysx

.type ppoll,@function
.size ppoll,.-ppoll
