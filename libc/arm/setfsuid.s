# auto-generated, remove this line before editing
.equ NR_setfsuid, 138

.text
.global setfsuid

setfsuid:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_setfsuid
	b	unisys

.type setfsuid,function
.size setfsuid,.-setfsuid
