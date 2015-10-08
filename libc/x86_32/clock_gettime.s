# auto-generated, remove this line before editing
.equ NR_clock_gettime, 228

.text
.global clock_gettime

clock_gettime:
	mov	$NR_clock_gettime, %al
	jmp	_syscall

.type clock_gettime,@function
.size clock_gettime,.-clock_gettime
