# auto-generated, remove this line before editing
.equ NR_getsockopt, 542

.text
.global getsockopt

getsockopt:
	mov	$NR_getsockopt, %ax
	jmp	_syscallx

.type getsockopt,@function
.size getsockopt,.-getsockopt
