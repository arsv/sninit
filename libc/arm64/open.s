.equ NR_openat, 56
.equ AT_FDCWD, -100

.text
.global open

open:
	mov	x3, x2		/* never used in init btw */
	mov	x2, x1
	mov	x1, x0
	mov	x0, AT_FDCWD
	mov	x8, NR_openat
	b	_syscall

.type stat,function
