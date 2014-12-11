# auto-generated, remove this line before editing
.equ NR_sigprocmask, 126

.text
.global sigprocmask

sigprocmask:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_sigprocmask
	b	unisys

.type sigprocmask,function
.size sigprocmask,.-sigprocmask
