# auto-generated, remove this line before editing
.equ NR_setrlimit, 164

.text
.global setrlimit

setrlimit:
	mov	x8, NR_setrlimit
	b	_syscall

.type setrlimit,function
.size setrlimit,.-setrlimit
