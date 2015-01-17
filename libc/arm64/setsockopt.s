# auto-generated, remove this line before editing
.equ NR_setsockopt, 208

.text
.global setsockopt

setsockopt:
	mov	x8, NR_setsockopt
	b	unisys

.type setsockopt,function
.size setsockopt,.-setsockopt
