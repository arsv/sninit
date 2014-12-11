# auto-generated, remove this line before editing
.equ NR_openat, 322

.text
.global openat

openat:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_openat
	b	unisys

.type openat,function
.size openat,.-openat
