# auto-generated, remove this line before editing
.equ NR_setfsgid, 152

.text
.global setfsgid

setfsgid:
	mov	x8, NR_setfsgid
	b	_syscall

.type setfsgid,function
.size setfsgid,.-setfsgid
