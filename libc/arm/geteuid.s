# auto-generated, remove this line before editing
.equ NR_geteuid, 49

.text
.global geteuid

geteuid:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_geteuid
	b	unisys

.type geteuid,function
.size geteuid,.-geteuid
