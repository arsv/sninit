.equ NR_openat, 56
.equ AT_FDCWD, -100

.text
.global open
.global openat

open:
	mov	x3, x2
	mov	x2, x1
	mov	x1, x0
	mov	x0, AT_FDCWD
openat:
	ldr	x8, =NR_openat
	b	unisys

.type openat,function
.size openat,.-openat

.type open,function
.size open,.-open
