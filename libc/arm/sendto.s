# auto-generated, remove this line before editing
.equ NR_sendto, 290

.text
.global sendto

sendto:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_sendto
	b	unisys6

.type sendto,function
.size sendto,.-sendto
