# auto-generated, remove this line before editing
.equ SYS_SENDMSG, 16

.text
.global sendmsg

sendmsg:
	movb $SYS_SENDMSG, %al
	jmp socketcall

.type sendmsg,@function
.size sendmsg,.-sendmsg
