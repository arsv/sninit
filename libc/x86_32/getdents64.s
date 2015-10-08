# auto-generated, remove this line before editing
.equ NR_getdents64, 217

.text
.global getdents64

getdents64:
	mov	$NR_getdents64, %al
	jmp	_syscall

.type getdents64,@function
.size getdents64,.-getdents64
