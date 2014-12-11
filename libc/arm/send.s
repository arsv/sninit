# auto-generated, remove this line before editing
.equ NR_send, 289

.text
.global send

send:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_send
	b	unisys

.type send,function
.size send,.-send
