# auto-generated, remove this line before editing
.equ NR_capset, 185

.text
.global capset

capset:
        stmfd	sp!,{r4,r5,r7,lr}
	ldr	r4, [sp,#16]
	ldr	r5, [sp,#20]
        ldr     r7, =NR_capset
	swi	0
	b	unisys

.type capset,function
.size capset,.-capset
