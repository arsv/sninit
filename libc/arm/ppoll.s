# auto-generated, remove this line before editing
.equ NR_ppoll, 336

.text
.global ppoll

ppoll:
        stmfd	sp!,{r4,r5,r7,lr}
	ldr	r4, [sp,#16]
	ldr	r5, [sp,#20]
        ldr     r7, =NR_ppoll
	swi	0
	b	unisys

.type ppoll,function
.size ppoll,.-ppoll
