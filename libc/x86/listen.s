# auto-generated, remove this line before editing
.equ SYS_LISTEN, 4

.text
.global listen

listen:
	movb $SYS_LISTEN, %al
	jmp socketcall

.type listen,@function
.size listen,.-listen
