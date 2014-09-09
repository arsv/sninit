#include <bits/syscall.h>

.text
.global time

time:
	mov	$__NR_time, %ax
	jmp	unisysx

.type time,@function
.size time,.-time
