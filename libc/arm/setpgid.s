# auto-generated, remove this line before editing
.equ NR_setpgid, 57

.text
.global setpgid

setpgid:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_setpgid
	b	unisys

.type setpgid,function
.size setpgid,.-setpgid
