# auto-generated, remove this line before editing
.equ NR_recvmsg, 297

.text
.global recvmsg

recvmsg:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r4, [sp,#16]
	ldr	r5, [sp,#20]
	ldr	r7, =NR_recvmsg
	b	unisys

.type recvmsg,function
.size recvmsg,.-recvmsg
