# auto-generated, remove this line before editing
.equ NR_clock_gettime, 265

.text
.global clock_gettime

clock_gettime:
	mov	$NR_clock_gettime, %ax
	jmp	_syscallx

.type clock_gettime,@function
.size clock_gettime,.-clock_gettime
