# auto-generated, remove this line before editing
.equ SYS_ACCEPT, 5

.text
.global accept

accept:
	movb $SYS_ACCEPT, %al
	jmp socketcall

.type accept,@function
.size accept,.-accept
