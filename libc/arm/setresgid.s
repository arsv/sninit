# auto-generated, remove this line before editing
.equ NR_setresgid, 170

.text
.global setresgid

setresgid:
        stmfd	sp!,{r4,r5,r7,lr}
	ldr	r4, [sp,#16]
	ldr	r5, [sp,#20]
        ldr     r7, =NR_setresgid
	swi	0
	b	unisys

.type setresgid,function
.size setresgid,.-setresgid