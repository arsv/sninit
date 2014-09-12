# auto-generated, remove this line before editing
.equ NR_setsockopt, 54

.text
.global setsockopt

setsockopt:
	mov	$NR_setsockopt, %al
	jmp	unisys

.type setsockopt,@function
.size setsockopt,.-setsockopt
