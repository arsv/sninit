# auto-generated, remove this line before editing
.equ NR_setresgid, 149

.text
.global setresgid

setresgid:
	mov	x8, NR_setresgid
	b	_syscall

.type setresgid,function
.size setresgid,.-setresgid
