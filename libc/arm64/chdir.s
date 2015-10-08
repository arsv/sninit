# auto-generated, remove this line before editing
.equ NR_chdir, 49

.text
.global chdir

chdir:
	mov	x8, NR_chdir
	b	_syscall

.type chdir,function
.size chdir,.-chdir
