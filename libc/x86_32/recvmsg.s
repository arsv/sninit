# auto-generated, remove this line before editing
.equ NR_recvmsg, 519

.text
.global recvmsg

recvmsg:
	mov	$NR_recvmsg, %ax
	jmp	unisysx

.type recvmsg,@function
.size recvmsg,.-recvmsg
