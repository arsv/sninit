.equ NR_dup3, 24

.text
.global dup2

dup2:
	mov	x2, #0
	ldr	x8, =NR_dup3
	b	unisys

.type dup2,function
.size dup2,.-dup2
