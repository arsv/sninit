# auto-generated, remove this line before editing
.equ NR_sendmsg, 296

.text
.global sendmsg

sendmsg:
        stmfd	sp!,{r4,r5,r7,lr}
	ldr	r4, [sp,#16]
	ldr	r5, [sp,#20]
        ldr     r7, =NR_sendmsg
	swi	0
	b	unisys

.type sendmsg,function
.size sendmsg,.-sendmsg
