# auto-generated, remove this line before editing
.equ NR_vfork, 190

.text
.global vfork

vfork:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_vfork
	b	unisys

.type vfork,function
.size vfork,.-vfork
