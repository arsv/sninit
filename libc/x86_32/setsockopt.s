# auto-generated, remove this line before editing
.equ NR_setsockopt, 541

.text
.global setsockopt

setsockopt:
	mov	$NR_setsockopt, %ax
	jmp	unisysx

.type setsockopt,@function
.size setsockopt,.-setsockopt
