# auto-generated, remove this line before editing
.equ NR_fstat, 108

.text
.global fstat

fstat:
	mov	$NR_fstat, %al
	jmp	_syscall

.type fstat,@function
.size fstat,.-fstat
