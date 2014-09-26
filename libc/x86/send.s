# auto-generated, remove this line before editing
.equ SYS_SEND, 9

.text
.global send

send:
	movb $SYS_SEND, %al
	jmp socketcall

.type send,@function
.size send,.-send
