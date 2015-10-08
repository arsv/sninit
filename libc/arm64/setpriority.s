# auto-generated, remove this line before editing
.equ NR_setpriority, 140

.text
.global setpriority

setpriority:
	mov	x8, NR_setpriority
	b	_syscall

.type setpriority,function
.size setpriority,.-setpriority
