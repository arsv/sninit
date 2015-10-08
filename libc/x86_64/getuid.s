# auto-generated, remove this line before editing
.equ NR_getuid, 102

.text
.global getuid

getuid:
	mov	$NR_getuid, %al
	jmp	_syscall

.type getuid,@function
.size getuid,.-getuid
