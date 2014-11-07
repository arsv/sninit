# auto-generated, remove this line before editing
.equ NR_sigaction, 67

.text
.global sigaction

sigaction:
        stmfd	sp!,{r4,r5,r7,lr}
	ldr	r4, [sp,#16]
	ldr	r5, [sp,#20]
        ldr     r7, =NR_sigaction
	swi	0
	b	unisys

.type sigaction,function
.size sigaction,.-sigaction
