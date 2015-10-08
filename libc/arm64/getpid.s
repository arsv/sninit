# auto-generated, remove this line before editing
.equ NR_getpid, 172

.text
.global getpid

getpid:
	mov	x8, NR_getpid
	b	_syscall

.type getpid,function
.size getpid,.-getpid
