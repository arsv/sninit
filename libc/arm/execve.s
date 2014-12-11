# auto-generated, remove this line before editing
.equ NR_execve, 11

.text
.global execve

execve:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_execve
	b	unisys

.type execve,function
.size execve,.-execve
