# auto-generated, remove this line before editing
.equ NR_setfsgid, 139

.text
.global setfsgid

setfsgid:
	mov	$NR_setfsgid, %al
	jmp	_syscall

.type setfsgid,@function
.size setfsgid,.-setfsgid
