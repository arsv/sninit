# auto-generated, remove this line before editing
.equ NR_getpid, 39

.text
.global getpid

getpid:
	mov	$NR_getpid, %al
	jmp	_syscall

.type getpid,@function
.size getpid,.-getpid
