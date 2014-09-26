# auto-generated, remove this line before editing
.equ SYS_SENDTO, 11

.text
.global sendto

sendto:
	movb $SYS_SENDTO, %al
	jmp socketcall

.type sendto,@function
.size sendto,.-sendto
