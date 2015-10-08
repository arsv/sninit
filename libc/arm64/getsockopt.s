# auto-generated, remove this line before editing
.equ NR_getsockopt, 209

.text
.global getsockopt

getsockopt:
	mov	x8, NR_getsockopt
	b	_syscall

.type getsockopt,function
.size getsockopt,.-getsockopt
