# auto-generated, remove this line before editing
.equ NR_recvmsg, 212

.text
.global recvmsg

recvmsg:
	mov	x8, NR_recvmsg
	b	unisys

.type recvmsg,function
.size recvmsg,.-recvmsg
