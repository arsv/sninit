# auto-generated, remove this line before editing
.equ NR_fcntl, 25

.text
.global fcntl

fcntl:
	mov	x8, NR_fcntl
	b	_syscall

.type fcntl,function
.size fcntl,.-fcntl
