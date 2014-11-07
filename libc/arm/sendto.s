# auto-generated, remove this line before editing
.equ NR_sendto, 290

.text
.global sendto

sendto:
        stmfd	sp!,{r4,r5,r7,lr}
	ldr	r4, [sp,#16]
	ldr	r5, [sp,#20]
        ldr     r7, =NR_sendto
	swi	0
	b	unisys

.type sendto,function
.size sendto,.-sendto
