#include "syscalls.h"

.text
.global waitpid

waitpid:
	xor	%rcx,%rcx
	mov	$__NR_wait4,%al
	jmp	unisys

.type waitpid,@function
.size waitpid,.-waitpid
