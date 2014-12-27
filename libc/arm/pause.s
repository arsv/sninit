# auto-generated, remove this line before editing
.equ NR_pause, 29

.text
.global pause

pause:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_pause
	b	unisys

.type pause,function
.size pause,.-pause
