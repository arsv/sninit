# auto-generated, remove this line before editing
.equ NR_bind, 49

.text
.global bind

bind:
	mov	$NR_bind, %al
	jmp	_syscall

.type bind,@function
.size bind,.-bind
