# auto-generated, remove this line before editing
.equ NR_getuid, 174

.text
.global getuid

getuid:
	mov	x8, NR_getuid
	b	_syscall

.type getuid,function
.size getuid,.-getuid
