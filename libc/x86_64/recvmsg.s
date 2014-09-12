# auto-generated, remove this line before editing
.equ NR_recvmsg, 47

.text
.global recvmsg

recvmsg:
	mov	$NR_recvmsg, %al
	jmp	unisys

.type recvmsg,@function
.size recvmsg,.-recvmsg
