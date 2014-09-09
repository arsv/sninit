#include <bits/syscall.h>

.text
.global time
.type time,@function

time:
	mov	$__NR_time, %ax
	jmp	__unified_syscall_16bit

.size time,.-time
