# auto-generated, remove this line before editing
.equ NR_shutdown, 293

.text
.global shutdown

shutdown:
        stmfd	sp!,{r4,r5,r7,lr}
	ldr	r4, [sp,#16]
	ldr	r5, [sp,#20]
        ldr     r7, =NR_shutdown
	swi	0
	b	unisys

.type shutdown,function
.size shutdown,.-shutdown
