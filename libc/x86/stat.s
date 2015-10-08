# auto-generated, remove this line before editing
.equ NR_stat, 106

.text
.global stat

stat:
	mov	$NR_stat, %al
	jmp	_syscall

.type stat,@function
.size stat,.-stat
