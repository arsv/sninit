#include <bits/syscall.h>

.equ sigbytes, 8		// _NSIG/8

.text
.global ppoll
ppoll:
	mov	$__NR_ppoll, %ax
	mov	$sigbytes, %r8d
	jmp	__unified_syscall_16bit

.type ppoll,@function
.size ppoll, .-ppoll
