# auto-generated, remove this line before editing
.equ NR_setsid, 66

.text
.global setsid

setsid:
        stmfd	sp!,{r4,r5,r7,lr}
	ldr	r4, [sp,#16]
	ldr	r5, [sp,#20]
        ldr     r7, =NR_setsid
	swi	0
	b	unisys

.type setsid,function
.size setsid,.-setsid
