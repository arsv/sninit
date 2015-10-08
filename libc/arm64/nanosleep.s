# auto-generated, remove this line before editing
.equ NR_nanosleep, 101

.text
.global nanosleep

nanosleep:
	mov	x8, NR_nanosleep
	b	_syscall

.type nanosleep,function
.size nanosleep,.-nanosleep
