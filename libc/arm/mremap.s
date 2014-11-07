# auto-generated, remove this line before editing
.equ NR_mremap, 163

.text
.global mremap

mremap:
        stmfd	sp!,{r4,r5,r7,lr}
	ldr	r4, [sp,#16]
	ldr	r5, [sp,#20]
        ldr     r7, =NR_mremap
	swi	0
	b	unisys

.type mremap,function
.size mremap,.-mremap
