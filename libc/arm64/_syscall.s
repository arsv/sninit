.data
.global errno

errno:	.word 0

.type errno,object
.size errno,.-errno


.text
.align 4

.global _syscall

_syscall:
	svc	0

	add	w1, w0, #0x1, lsl #12
	cmp	w1, #0xfff
	b.ls	1f
	ret	/* no error */

1:	adr	x1, errno
	neg	w0, w0
	str	w0, [x1]
	mov	x0, #-1
	ret	/* error */

.size _syscall,.-_syscall
.type _syscall,function
