# auto-generated, remove this line before editing
.equ SYS_GETSOCKOPT, 15

.text
.global getsockopt

getsockopt:
	movb $SYS_GETSOCKOPT, %al
	jmp socketcall

.type getsockopt,@function
.size getsockopt,.-getsockopt
