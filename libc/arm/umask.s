# auto-generated, remove this line before editing
.equ NR_umask, 60

.text
.global umask

umask:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_umask
	b	unisys

.type umask,function
.size umask,.-umask
