# auto-generated, remove this line before editing
.equ NR_sigaction, 67

.text
.global sigaction

sigaction:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_sigaction
	b	unisys

.type sigaction,function
.size sigaction,.-sigaction
