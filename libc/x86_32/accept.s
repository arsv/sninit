# auto-generated, remove this line before editing
.equ NR_accept, 43

.text
.global accept

accept:
	mov	$NR_accept, %al
	jmp	unisys

.type accept,@function
.size accept,.-accept
