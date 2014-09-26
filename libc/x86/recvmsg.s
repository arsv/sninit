# auto-generated, remove this line before editing
.equ SYS_RECVMSG, 17

.text
.global recvmsg

recvmsg:
	movb $SYS_RECVMSG, %al
	jmp socketcall

.type recvmsg,@function
.size recvmsg,.-recvmsg
