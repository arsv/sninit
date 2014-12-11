# auto-generated, remove this line before editing
.equ NR_sendmsg, 296

.text
.global sendmsg

sendmsg:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_sendmsg
	b	unisys

.type sendmsg,function
.size sendmsg,.-sendmsg
