# auto-generated, remove this line before editing
.equ SYS_SETSOCKOPT, 14

.text
.global setsockopt

setsockopt:
	movb $SYS_SETSOCKOPT, %al
	jmp socketcall

.type setsockopt,@function
.size setsockopt,.-setsockopt
